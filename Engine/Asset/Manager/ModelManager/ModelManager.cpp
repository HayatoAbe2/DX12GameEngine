#include "ModelManager.h"
#include "Engine/Asset/Manager/TextureManager/TextureManager.h"
#include "Engine/SceneObject/Model/Model.h"
#include "Engine/SceneObject/Model/InstancedModel.h"
#include "Engine/SceneObject/Model/ModelData.h"

#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"
#include "Engine/Io/Logger/Logger.h"
#include "Engine/Graphics/GPUData/VertexData.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"

#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Engine/Graphics/GPUData/InstanceGPUData.h>

ModelManager::ModelManager(DirectXContext* dxContext, Logger* logger, TextureManager* textureManager) {
	// デバイス
	device_ = dxContext->GetDeviceManager()->GetDevice().Get();
	// CommandList管理クラス
	commandListManager_ = dxContext->GetCommandListManager();
	// DescriptorHeap管理クラス
	descriptorHeapManager_ = dxContext->GetDescriptorHeapManager();
	// SRV管理クラス
	descriptorManager_ = dxContext->GetSRVManager();
	// バッファ管理クラス
	bufferManager_ = dxContext->GetBufferManager();
	// ログ出力クラス
	logger_ = logger;
	// テクスチャ管理クラス
	textureManager_ = textureManager;
}

std::unique_ptr<Model> ModelManager::Load(uint32_t id, uint32_t textureId, uint32_t envTextureId, uint32_t materialId, const std::string& directoryPath, const std::string& filename) {
	std::unique_ptr<Model> model = std::make_unique<Model>(id); // 構築するModel
	std::shared_ptr<ModelData> modelData = std::make_shared<ModelData>(); // データ

	// ファイルパス
	std::string filePath = directoryPath + "/" + filename;
	model->SetDirectoryPath(directoryPath);

	// assimpでモデル作成
	Assimp::Importer importer;

	// obj->DirectX12変換
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);

	if (!scene) {
		std::string error = importer.GetErrorString();
		logger_->Log(logger_->GetStream(), error);
		assert(false && "Assimp failed to load model");
		return nullptr;
	}

	// rootNode
	std::unique_ptr<ModelNode> rootNode = std::move(ReadNode(scene->mRootNode));
	// スケルトン作成
	auto skeleton = CreateSkeleton(*rootNode.get());
	model->SetRootNode(std::move(rootNode));
	model->SetSkeleton(skeleton);

	// キャッシュにあるか確認
	std::string fullPath = directoryPath + "/" + filename;
	auto it = modelDataCache_.find(fullPath);
	if (it != modelDataCache_.end()) {
		// あった場合、マテリアルを作成しなおす
		auto cacheData = it->second;
		model->CopyModelData(cacheData, bufferManager_);

		// メッシュ数設定
		std::vector<MeshRuntime> meshes;
		meshes.resize(scene->mNumMeshes);

		// メッシュ
		for (uint32_t aiMeshIndex = 0; aiMeshIndex < scene->mNumMeshes; ++aiMeshIndex) {
			aiMesh* aiMesh = scene->mMeshes[aiMeshIndex];

			// skinCluster用jointWeight
			for (uint32_t boneIndex = 0; boneIndex < aiMesh->mNumBones; ++boneIndex) {
				aiBone* bone = aiMesh->mBones[boneIndex];
				std::string jointName = bone->mName.C_Str();
				JointWeightData& jointWeightData = modelData->JointWeights[jointName];

				aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
				aiVector3D scale, translate;
				aiQuaternion rotate;
				bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
				// 左手系へ変換
				Matrix4x4 binePoseMatrix = MakeAffineMatrix({ scale.x, scale.y, scale.z },
					{ rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
				jointWeightData.InverseBindPoseMatrix = Inverse(binePoseMatrix);

				// 頂点Weightを取得
				for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
					jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
				}
			}

			// subMesh生成
			SubMeshRuntime subMeshRuntime = CreateSubMesh(aiMesh);

			// メッシュにsubMeshを追加
			meshes[aiMeshIndex].subMeshes.push_back(subMeshRuntime);
		}
		model->SetSkinCluster(CreateSkinCluster(skeleton, cacheData.get()));
		model->SetMeshes(meshes);
	} else {
		// メッシュ数設定
		std::vector<MeshRuntime> meshes;
		meshes.resize(scene->mNumMeshes);
		modelData->meshes.resize(scene->mNumMeshes);

		// メッシュ
		for (uint32_t aiMeshIndex = 0; aiMeshIndex < scene->mNumMeshes; ++aiMeshIndex) {
			aiMesh* aiMesh = scene->mMeshes[aiMeshIndex];

			// skinCluster用jointWeight
			for (uint32_t boneIndex = 0; boneIndex < aiMesh->mNumBones; ++boneIndex) {
				aiBone* bone = aiMesh->mBones[boneIndex];
				std::string jointName = bone->mName.C_Str();
				JointWeightData& jointWeightData = modelData->JointWeights[jointName];

				aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
				aiVector3D scale, translate;
				aiQuaternion rotate;
				bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
				// 左手系へ変換
				Matrix4x4 binePoseMatrix = MakeAffineMatrix({ scale.x, scale.y, scale.z },
					{ rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
				jointWeightData.InverseBindPoseMatrix = Inverse(binePoseMatrix);

				// 頂点Weightを取得
				for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
					jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
				}
			}

			// subMesh生成
			SubMeshRuntime subMeshRuntime = CreateSubMesh(aiMesh);
			SubMeshData subMeshData = CreateSubMeshData(aiMesh);

			// メッシュにsubMeshを追加
			meshes[aiMeshIndex].subMeshes.push_back(subMeshRuntime);
			modelData->meshes[aiMeshIndex].subMeshes.push_back(subMeshData);
		}
		model->SetSkinCluster(CreateSkinCluster(skeleton, modelData.get()));

		model->SetMeshes(meshes);

		///
		/// マテリアルの設定
		/// 

		for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
			auto texture = std::make_shared<Texture>(textureId);

			aiMaterial* aiMaterial = scene->mMaterials[materialIndex];
			const aiTexture* tex = nullptr;
			if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
				aiString textureFilePath;
				aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

				texture->SetMtlFilePath(directoryPath + "/" + textureFilePath.C_Str());
				if (filePath.ends_with(".FBX") || filePath.ends_with(".GLB")) {
					const aiTexture* tex = scene->GetEmbeddedTexture(textureFilePath.C_Str());

					// SRVを作成
					textureManager_->CreateTextureSRV(texture, tex);
				} else {
					textureManager_->CreateTextureSRV(texture);
				}

			} else {
				texture->SetMtlFilePath("Resources/Debug/white1x1.png");
				textureManager_->CreateTextureSRV(texture);
			}

			// デフォルト環境テクスチャ
			auto environmentTexture = std::make_shared<Texture>(envTextureId);
			environmentTexture->SetMtlFilePath("Resources/Debug/rostock_laage_airport_4k.dds");
			textureManager_->CreateTextureSRV(environmentTexture);

			// マテリアル初期化
			std::unique_ptr<Material> material = std::make_unique<Material>(materialId);
			bool useTexture = !texture->GetMtlPath().empty();
			material->Initialize(bufferManager_, useTexture, true); // テクスチャ座標情報がなければテクスチャ不使用
			material->SetTexture(texture);	// テクスチャ
			material->SetEnvironmentTexture(environmentTexture);

			aiColor4D baseColor(1, 1, 1, 1);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_BASE_COLOR, &baseColor);

			auto data = material->GetData();
			data.color = {
				baseColor.r,
				baseColor.g,
				baseColor.b,
				baseColor.a
			};
			material->SetData(data);

			// モデルデータ作成
			modelData->defaultMaterials_.push_back(std::move(material));
		}

		// モデルにデータの参照
		model->CopyModelData(modelData, bufferManager_);

		// キャッシュに登録
		modelDataCache_.insert({ fullPath, modelData });
	}

	return model;
}

std::unique_ptr<InstancedModel> ModelManager::Load(uint32_t id, uint32_t textureId, uint32_t envTextureId, uint32_t materialId, const std::string& directoryPath, const std::string& filename, const int numInstance) {
	if (numInstance == 0) { assert(false); } // インスタンス数0の場合止める

	std::unique_ptr<InstancedModel> model = std::make_unique<InstancedModel>(id); // 構築するModel
	std::shared_ptr<ModelData> modelData = std::make_shared<ModelData>(); // データ

	// assimpでモデル作成
	Assimp::Importer importer;
	// ファイルパス
	std::string filePath = directoryPath + "/" + filename;
	model->SetDirectoryPath(directoryPath);
	// obj->DirectX12変換
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);

	// rootNode
	std::unique_ptr<ModelNode> rootNode = std::move(ReadNode(scene->mRootNode));
	Skeleton skeleton = CreateSkeleton(*rootNode.get());
	model->SetRootNode(std::move(rootNode));

	// キャッシュにあるか確認
	std::string fullPath = directoryPath + "/" + filename;
	auto it = modelDataCache_.find(fullPath);
	if (it != modelDataCache_.end()) {

		// あった場合、マテリアルを作成しなおす
		auto cacheData = it->second;
		model->CopyModelData(cacheData, bufferManager_, cbManager_);

		model->SetNumInstance(numInstance);
		// インスタンス数分のtransformリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformResource = bufferManager_->CreateUploadBuffer(sizeof(InstanceGPUData) * numInstance);
		InstanceGPUData* transformData = nullptr;
		instanceTransformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));
		// 単位行列を書き込んでおく
		for (int i = 0; i < numInstance; ++i) {
			transformData[i].World = MakeIdentity4x4();
			transformData[i].WVP = MakeIdentity4x4();
			transformData[i].WorldInverseTranspose = MakeIdentity4x4();
			model->AddInstanceTransform();
		}

		model->SetInstanceResource(instanceTransformResource);
		model->SetInstanceTransformData(transformData);
		CreateInstancingSRV(model.get(), numInstance);
		return model; // キャッシュにあったのでそれを返す 
	}

	// メッシュ数設定
	std::vector<MeshRuntime> meshes;
	meshes.resize(scene->mNumMeshes);
	modelData->meshes.resize(scene->mNumMeshes);

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		// 新規メッシュ
		std::unique_ptr<MeshRuntime> mesh = std::make_unique<MeshRuntime>();

		aiMesh* aiMesh = scene->mMeshes[meshIndex];
		assert(aiMesh->HasNormals());
		assert(aiMesh->HasTextureCoords(0));

		// subMesh
		SubMeshRuntime subMesh = CreateSubMesh(aiMesh);
		SubMeshData subMeshData = CreateSubMeshData(aiMesh);

		// メッシュにsubMeshを追加
		meshes[meshIndex].subMeshes.push_back(subMesh);
		modelData->meshes[meshIndex].subMeshes.push_back(subMeshData);
	}

	///
	/// マテリアルの設定
	/// 

	std::shared_ptr<Texture> texture = std::make_shared<Texture>(textureId);	// テクスチャ

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* aiMaterial = scene->mMaterials[materialIndex];
		if (aiMaterial->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			// テクスチャ設定
			texture->SetMtlFilePath(directoryPath + "/" + textureFilePath.C_Str());
		} else {
			texture->SetMtlFilePath("Resources/Debug/white1x1.png");
		}

		// SRVを作成
		textureManager_->CreateTextureSRV(texture);

		// デフォルト環境テクスチャ
		auto environmentTexture = std::make_shared<Texture>(envTextureId);
		environmentTexture->SetMtlFilePath("Resources/Debug/rostock_laage_airport_4k.dds");
		textureManager_->CreateTextureSRV(environmentTexture);

		// マテリアル初期化
		std::unique_ptr<Material> material = std::make_unique<Material>(materialId);
		bool useTexture = !texture->GetMtlPath().empty();
		material->Initialize(bufferManager_, useTexture, true); // テクスチャ座標情報がなければテクスチャ不使用
		material->SetTexture(texture);	// テクスチャ
		material->SetEnvironmentTexture(environmentTexture);

		aiColor4D baseColor(1, 1, 1, 1);
		aiGetMaterialColor(aiMaterial, AI_MATKEY_BASE_COLOR, &baseColor);

		auto data = material->GetData();
		data.color = {
			baseColor.r,
			baseColor.g,
			baseColor.b,
			baseColor.a
		};
		material->SetData(data);

		// モデルデータ作成
		modelData->defaultMaterials_.push_back(std::move(material));
	}

	// モデルにデータの参照
	model->CopyModelData(modelData, bufferManager_, cbManager_);

	// キャッシュに登録
	modelDataCache_.insert({ fullPath, std::move(modelData) });

	///
	/// インスタンシング用の設定
	///

	model->SetNumInstance(numInstance);
	// インスタンス数分のtransformリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformResource = bufferManager_->CreateUploadBuffer(sizeof(InstanceGPUData) * numInstance);
	InstanceGPUData* transformData = nullptr;
	instanceTransformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));
	// 単位行列を書き込んでおく
	for (int i = 0; i < numInstance; ++i) {
		transformData[i].World = MakeIdentity4x4();
		transformData[i].WVP = MakeIdentity4x4();
		transformData[i].WorldInverseTranspose = MakeIdentity4x4();
		model->AddInstanceTransform();
	}

	model->SetInstanceResource(instanceTransformResource);
	model->SetInstanceTransformData(transformData);
	CreateInstancingSRV(model.get(), numInstance);

	return model;
}

SubMeshRuntime ModelManager::CreateSubMesh(aiMesh* aiMesh) {
	SubMeshRuntime subMesh{};

	size_t size = sizeof(VertexData) * aiMesh->mNumVertices;

	// ComputeShader出力用VB
	subMesh.outputVertexBuffer_ = bufferManager_->CreateDefaultBuffer(size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	subMesh.outputVertexUAVIndex = descriptorManager_->Allocate();
	descriptorManager_->CreateStructuredBufferUAV(subMesh.outputVertexUAVIndex, subMesh.outputVertexBuffer_.Get(), UINT(aiMesh->mNumVertices), sizeof(VertexData));
	subMesh.outputVBV_.BufferLocation = subMesh.outputVertexBuffer_->GetGPUVirtualAddress();
	subMesh.outputVBV_.SizeInBytes = UINT(size);
	subMesh.outputVBV_.StrideInBytes = sizeof(VertexData);

	return subMesh;
}

SubMeshData ModelManager::CreateSubMeshData(aiMesh* aiMesh) {
	SubMeshData data;
	data.materialIndex_ = aiMesh->mMaterialIndex;

	///
	/// index
	///
	for (uint32_t faceIndex = 0; faceIndex < aiMesh->mNumFaces; ++faceIndex) {
		aiFace& face = aiMesh->mFaces[faceIndex];

		assert(face.mNumIndices == 3); // 三角形
		for (uint32_t i = 0; i < face.mNumIndices; ++i) {
			data.indices_.push_back(face.mIndices[i]);
		}
	}

	// IBV
	size_t indexSize = sizeof(uint32_t) * data.indices_.size();
	data.indexBuffer_ = bufferManager_->CreateUploadBuffer(indexSize);

	uint32_t* dst = nullptr;
	data.indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dst));
	memcpy(dst, data.indices_.data(), indexSize);
	data.indexBuffer_->Unmap(0, nullptr);

	data.ibv_.BufferLocation = data.indexBuffer_->GetGPUVirtualAddress();
	data.ibv_.SizeInBytes = UINT(indexSize);
	data.ibv_.Format = DXGI_FORMAT_R32_UINT;

	///
	/// メッシュ頂点の設定
	/// 

	data.vertices_.resize(aiMesh->mNumVertices);
	for (uint32_t v = 0; v < aiMesh->mNumVertices; ++v) {
		aiVector3D& position = aiMesh->mVertices[v];
		// 頂点データ
		VertexData vertex;
		vertex.position = { position.x,position.y,position.z, 1.0f };
		vertex.position.x *= -1.0f; // 左手系

		// 法線
		if (aiMesh->HasNormals()) {
			aiVector3D& normal = aiMesh->mNormals[v];
			vertex.normal = { normal.x,normal.y,normal.z };
			vertex.normal.x *= -1.0f;
		} else {
			// 法線がない場合
			vertex.normal = { 0,1,0 };
		}

		// TexCoord
		if (aiMesh->HasTextureCoords(0)) {
			aiVector3D& texcoord = aiMesh->mTextureCoords[0][v];
			vertex.texcoord = { texcoord.x, texcoord.y };
		} else {
			vertex.texcoord = {};
		}

		if (aiMesh->HasVertexColors(0)) {
			// 頂点カラー
			aiColor4D& color = aiMesh->mColors[0][v];
			vertex.color = { color.r, color.g, color.b, color.a };
		} else {
			// なかったら白色
			vertex.color = { 1.0f,1.0f,1.0f,1.0f };
		}

		data.vertices_[v] = vertex;
	}

	size_t size = sizeof(VertexData) * data.vertices_.size();

	// 入力VertexBuffer
	data.vertexBuffer_ = bufferManager_->CreateUploadBuffer(size);
	VertexData* dst2 = nullptr;
	data.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dst2));
	memcpy(dst2, data.vertices_.data(), size);
	data.vertexBuffer_->Unmap(0, nullptr);
	data.inputVertexSRVIndex = descriptorManager_->Allocate();
	descriptorManager_->CreateStructuredBufferSRV(data.inputVertexSRVIndex, data.vertexBuffer_.Get(), UINT(data.vertices_.size()), sizeof(VertexData));

	// VBV
	data.vertexBufferView_.BufferLocation = data.vertexBuffer_->GetGPUVirtualAddress();
	data.vertexBufferView_.SizeInBytes = UINT(size);
	data.vertexBufferView_.StrideInBytes = sizeof(VertexData);

	return data;
}

void ModelManager::CreateInstancingSRV(InstancedModel* model, const int numInstance_) {
	auto index = descriptorManager_->Allocate();
	descriptorManager_->CreateStructuredBufferSRV(index, model->GetInstanceResource().Get(), numInstance_, sizeof(InstanceGPUData));

	// SRVハンドル
	model->SetSRVHandle(descriptorManager_->GetGPUHandle(index));
}

std::unique_ptr<ModelNode> ModelManager::ReadNode(aiNode* aiNode) {
	std::unique_ptr<ModelNode> node = std::make_unique<ModelNode>();

	aiVector3D scale, translate;
	aiQuaternion rotate;
	// SRT
	aiNode->mTransformation.Decompose(scale, rotate, translate);
	// 左手系に変換して使用
	node->transform.scale = { scale.x, scale.y, scale.z };
	node->transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w };
	node->transform.translate = { -translate.x, translate.y, translate.z };
	node->localMatrix = MakeAffineMatrix(node->transform.scale, node->transform.rotate, node->transform.translate);

	// meshのindex
	node->meshIndices.reserve(aiNode->mNumMeshes);
	for (uint32_t i = 0; i < aiNode->mNumMeshes; ++i) {
		node->meshIndices.push_back(aiNode->mMeshes[i]);
	}

	// 子ノード
	node->children.reserve(aiNode->mNumChildren);
	for (uint32_t i = 0; i < aiNode->mNumChildren; ++i) {
		// 階層構造
		node->children.push_back(ReadNode(aiNode->mChildren[i]));
	}

	// name
	node->name = aiNode->mName.C_Str();

	return node;
}

Matrix4x4 ModelManager::ConvertAssimpMatrixToLHRow(const aiMatrix4x4& m) {
	Matrix4x4 out{};

	// 列から行ベクトル、右手から左手座標系に
	out.m[0][0] = m.a1; out.m[0][1] = m.b1; out.m[0][2] = -m.c1; out.m[0][3] = m.d1;
	out.m[1][0] = m.a2; out.m[1][1] = m.b2; out.m[1][2] = -m.c2; out.m[1][3] = m.d2;
	out.m[2][0] = -m.a3; out.m[2][1] = -m.b3; out.m[2][2] = m.c3; out.m[2][3] = -m.d3;
	out.m[3][0] = m.a4; out.m[3][1] = m.b4; out.m[3][2] = -m.c4; out.m[3][3] = m.d4;

	return out;
}

std::unique_ptr<ParticleSystem> ModelManager::CreateParticleInstanceResource(int numInstance, uint32_t id) {
	std::unique_ptr<ParticleSystem> particleSystem = std::make_unique<ParticleSystem>(id);

	// インスタンス数分のtransformリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformResource = bufferManager_->CreateUploadBuffer(sizeof(InstanceGPUData) * numInstance);
	InstanceGPUData* transformData = nullptr;
	instanceTransformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));
	// 単位行列を書き込んでおく
	for (int i = 0; i < numInstance; ++i) {
		transformData[i].World = MakeIdentity4x4();
		transformData[i].WVP = MakeIdentity4x4();
		transformData[i].WorldInverseTranspose = MakeIdentity4x4();
	}

	particleSystem->SetInstanceResource(instanceTransformResource);
	particleSystem->SetInstanceTransformData(transformData);

	auto index = descriptorManager_->Allocate();
	descriptorManager_->CreateStructuredBufferSRV(index, particleSystem->GetInstanceResource().Get(), numInstance, sizeof(InstanceGPUData));
	// SRVハンドル
	particleSystem->SetSRVHandle(descriptorManager_->GetGPUHandle(index));

	return std::move(particleSystem);
}

std::unique_ptr<Primitive> ModelManager::CreatePrimitive(uint32_t id) {
	std::unique_ptr<Primitive> primitive = std::make_unique<Primitive>(id);
	return std::move(primitive);
}

std::unique_ptr<Material> ModelManager::LoadMaterial(std::shared_ptr<Texture> texture, uint32_t id, uint32_t textureId) {
	// マテリアル初期化
	std::unique_ptr<Material> material = std::make_unique<Material>(id);
	bool useTexture = !texture->GetMtlPath().empty();
	material->Initialize(bufferManager_, useTexture, true); // テクスチャ座標情報がなければテクスチャ不使用
	material->SetTexture(texture);	// テクスチャ

	// デフォルト環境テクスチャ
	auto environmentTexture = std::make_shared<Texture>(textureId);
	environmentTexture->SetMtlFilePath("Resources/Debug/rostock_laage_airport_4k.dds");
	textureManager_->CreateTextureSRV(environmentTexture);
	material->SetEnvironmentTexture(environmentTexture);

	return std::move(material);
}

Skeleton ModelManager::CreateSkeleton(const ModelNode& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とindexをマッピング
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

int32_t ModelManager::CreateJoint(const ModelNode& node,
	const std::optional<int32_t>& parent,
	std::vector<Joint>& joints) {
	// Jointを作成して追加
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());
	joint.parent = parent;
	joints.push_back(joint);

	// 再帰呼び出しで子のjointを作成、登録
	for (auto& child : node.children) {
		int32_t childIndex = CreateJoint(*child.get(), joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	// 自身のindex
	return joint.index;
}

SkinClusterRuntime ModelManager::CreateSkinCluster(const Skeleton& skeleton, ModelData* data) {
	auto index = descriptorManager_->Allocate();
	SkinClusterRuntime skinCluster;
	skinCluster.paletteResource = bufferManager_->CreateUploadBuffer(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette, skeleton.joints.size() };
	skinCluster.paletteSrvHandle.first = descriptorManager_->GetCPUHandle(index);
	skinCluster.paletteSrvHandle.second = descriptorManager_->GetGPUHandle(index);
	descriptorManager_->CreateStructuredBufferSRV(index, skinCluster.paletteResource.Get(), UINT(skeleton.joints.size()), sizeof(WellForGPU));

	for (auto& mesh : data->meshes) {
		for (auto& subMesh : mesh.subMeshes) {
			// Influence
			data->skinClusterData_.influenceResource = bufferManager_->CreateUploadBuffer(sizeof(VertexInfluence) * subMesh.vertices_.size());
			VertexInfluence* mappedInfluence = nullptr;
			data->skinClusterData_.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
			std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * subMesh.vertices_.size());
			data->skinClusterData_.mappedInfluence = { mappedInfluence, subMesh.vertices_.size() };

			// InverseBindPoseMatrixの格納
			data->skinClusterData_.inverseBindPoseMatrices.resize(skeleton.joints.size());
			std::generate(
				data->skinClusterData_.inverseBindPoseMatrices.begin(),
				data->skinClusterData_.inverseBindPoseMatrices.end(),
				[]() {
					return MakeIdentity4x4();
				});

			for (const auto& jointWeight : data->JointWeights) {
				auto it = skeleton.jointMap.find(jointWeight.first);
				if (it == skeleton.jointMap.end()) { continue; }

				data->skinClusterData_.inverseBindPoseMatrices[(*it).second] = jointWeight.second.InverseBindPoseMatrix;
				for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
					auto& currentInfluence = data->skinClusterData_.mappedInfluence[vertexWeight.vertexIndex];
					for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
						if (currentInfluence.weights[index] == 0.0f) {
							currentInfluence.weights[index] = vertexWeight.weight;
							currentInfluence.jointIndices[index] = (*it).second;
							break;
						}
					}
				}
			}
			skinCluster.paletteSRVIndex = descriptorManager_->Allocate();
			descriptorManager_->CreateStructuredBufferSRV(skinCluster.paletteSRVIndex, skinCluster.paletteResource.Get(), UINT(skeleton.joints.size()), sizeof(WellForGPU));

			data->skinClusterData_.influenceSRVIndex = descriptorManager_->Allocate();
			descriptorManager_->CreateStructuredBufferSRV(data->skinClusterData_.influenceSRVIndex, data->skinClusterData_.influenceResource.Get(), UINT(subMesh.vertices_.size()), sizeof(VertexInfluence));

			subMesh.skinningInformationBuffer = bufferManager_->CreateUploadBuffer(sizeof(SkinningInformation));
			SkinningInformation skinningInformation;
			skinningInformation.numVertices = uint32_t(subMesh.vertices_.size());

			void* mapped = nullptr;
			subMesh.skinningInformationBuffer->Map(0, nullptr, &mapped);
			memcpy(mapped, &skinningInformation, sizeof(skinningInformation));
			subMesh.skinningInformationBuffer->Unmap(0, nullptr);
		}
	}

	return skinCluster;
}