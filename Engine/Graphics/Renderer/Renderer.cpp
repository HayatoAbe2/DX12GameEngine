#include <Windows.h>
#include "Renderer.h"
#include "Engine/Graphics/Core/CommandListManager/CommandListManager.h"
#include "Engine/Graphics/Pipeline/PipelineStateManager/PipelineStateManager.h"
#include "Engine/Graphics/Pipeline/RootSignatureManager/RootSignatureManager.h"
#include "Engine/Graphics/Core/DeviceManager/DeviceManager.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"
#include "Engine/Asset/Model/Model.h"
#include "Engine/Asset/Model/InstancedModel.h"
#include "Engine/Asset/Sprite/Sprite.h"
#include "Engine/Object/Particle/ParticleSystem/ParticleSystem.h"
#include "Engine/Scene/Camera/Camera.h"
#include <numbers>

void Renderer::Initialize(DirectXContext* dxContext) {
	dxContext_ = dxContext;

	// デバイス
	auto device = dxContext_->GetDeviceManager()->GetDevice();

	// CB管理クラス
	cbManager_ = dxContext->GetConstantBufferManager();

	// カメラバッファ作成
	cameraBuffer_ = dxContext_->GetBufferManager()->CreateUploadBuffer(sizeof(CameraForGPU));
	cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	// ライト非使用時のダミー
	dummyLightBuffer_ = dxContext_->GetBufferManager()->CreateUploadBuffer(sizeof(LightsForGPU));
	dummyLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dummyLight_));

	InitializePlane();
	InitializeRing();
	InitializeSkybox();
}

void Renderer::UpdateSpriteTransform(Sprite* sprite) {
	Transform transform{};
	Vector2 size = sprite->GetSize();
	Vector2 pos = sprite->GetPosition();
	float rot = sprite->GetRotation();
	Vector2 windowSize = { float(dxContext_->GetWindowWidth()),float(dxContext_->GetWindowHeight()) };

	transform.scale = { size.x, size.y, 1.0f };
	transform.translate = { pos.x,pos.y, 0.0f };
	transform.rotate = { 0.0f,0.0f,rot };
	// トランスフォーム
	TransformationMatrix data;
	data.World = MakeAffineMatrix(transform);
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, windowSize.x, windowSize.y, 0.0f, 100.0f);
	data.WVP = Multiply(data.World, projectionMatrix);
	data.WorldInverseTranspose = Transpose(Inverse(data.World));

	auto cbAddress = dxContext_->GetConstantBufferManager()->UploadTransform(data); // gpu送信
	// トランスフォームCBV
	dxContext_->GetCommandListManager()->GetCommandList()->SetGraphicsRootConstantBufferView(1, cbAddress);
}

void Renderer::DrawModel(Model* model, LightManager* lightManager, int blendMode) {
	// GPUに渡すデータの更新
	cameraData_->position = camera_->transform_.translate;
	if (lightManager) { lightManager->Update(); }

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetStandardPSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetStandardRootSignature().Get();

	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// カメラ
	cmdList->SetGraphicsRootConstantBufferView(4, cameraBuffer_->GetGPUVirtualAddress());
	// ライト
	cmdList->SetGraphicsRootConstantBufferView(5, lightManager->GetLightResource()->GetGPUVirtualAddress());

	DrawNode(model, cmdList, model->GetRootNode(), MakeIdentity4x4());
}

void Renderer::DrawModelInstance(InstancedModel* model, LightManager* lightManager, int blendMode) {
	// GPUに渡すデータの更新
	cameraData_->position = camera_->transform_.translate;
	if (lightManager) { lightManager->Update(); }

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetInstancingPSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetInstancingRootSignature().Get();

	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// wvp用のCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(1, model->GetInstanceCBV());
	// インスタンス用SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(4, model->GetInstanceSRVHandle());
	// カメラ
	cmdList->SetGraphicsRootConstantBufferView(5, cameraBuffer_->GetGPUVirtualAddress());
	if (lightManager) {
		// ライト
		cmdList->SetGraphicsRootConstantBufferView(6, lightManager->GetLightResource()->GetGPUVirtualAddress());
	} else {
		cmdList->SetGraphicsRootConstantBufferView(6, dummyLightBuffer_->GetGPUVirtualAddress());
	}

	DrawNodeInstance(model, cmdList, model->GetRootNode(), MakeIdentity4x4());
}

void Renderer::DrawParticles(ParticleSystem* particleSys, int blendMode) {
	particleSys->PreDraw(camera_);

	Material* material = particleSys->GetMaterial();
	// マテリアル更新
	material->UpdateGPU();

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetParticlePSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetParticleRootSignature().Get();

	cmdList->SetPipelineState(pso);
	cmdList->SetGraphicsRootSignature(rootSig);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT indexCountPerInstance = 0;
	// 各図形のVB・IB
	switch (particleSys->GetShape()) {
	case ParticleShape::Plane:
		cmdList->IASetVertexBuffers(0, 1, &plane_.vbv);
		cmdList->IASetIndexBuffer(&plane_.ibv);
		indexCountPerInstance = 6;
		break;
	case ParticleShape::Ring:
		cmdList->IASetVertexBuffers(0, 1, &ring_.vbv);
		cmdList->IASetIndexBuffer(&ring_.ibv);
		indexCountPerInstance = 6 * 32;
		break;
	}

	// マテリアルCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(1, particleSys->GetInstanceCBV());
	// SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
	// インスタンス用SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(3, particleSys->GetInstanceSRVHandle());
	// ドローコール
	cmdList->DrawIndexedInstanced(indexCountPerInstance, particleSys->GetNumInstance(), 0, 0, 0);
}

void Renderer::DrawSprite(Sprite* sprite, int blendMode) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetSpritePSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetStandardRootSignature().Get();

	sprite->UpdateMaterial();
	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UpdateSpriteTransform(sprite);
	// マテリアルCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(0, sprite->GetMaterialCBV());
	// Spriteの描画。変更が必要なものだけ変更する
	cmdList->IASetIndexBuffer(&sprite->GetIBV());	// IBVを設定
	cmdList->IASetVertexBuffers(0, 1, &sprite->GetVBV());	// VBVを設定
	// SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(2, sprite->GetTextureSRVHandle());
	// ドローコール
	cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Renderer::DrawNode(Model* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld) {
	Matrix4x4 modelWorld = MakeAffineMatrix(model->GetTransform());
	Matrix4x4 nodeWorld = node->localMatrix * parentWorld;

	// トランスフォーム更新
	TransformationMatrix data;
	data.World = modelWorld;
	data.WVP = data.World
		* camera_->viewMatrix_
		* camera_->projectionMatrix_;
	data.WorldInverseTranspose = Transpose(Inverse(data.World));
	auto cbAddress = dxContext_->GetConstantBufferManager()->UploadTransform(data); // gpu送信
	cmdList->SetGraphicsRootConstantBufferView(1, cbAddress);

	// メッシュを描画
	for (uint32_t meshIndex : node->meshIndices) {
		DrawMesh(model, model->GetData()->meshes[meshIndex].get());
	}

	// 子ノード
	for (auto& child : node->children) {
		DrawNode(model, cmdList, child.get(), nodeWorld);
	}
}

void Renderer::DrawMesh(Model* model, Mesh* mesh) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();

	for (const auto& subMesh : mesh->GetPrimitives()) {
		Material* material = model->GetMaterial(subMesh.materialIndex_);

		// マテリアル更新
		material->UpdateGPU();

		// マテリアルCBufferの場所を設定
		cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
		// メッシュVBV
		cmdList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView_);	// VBVを設定
		// IBV
		cmdList->IASetIndexBuffer(&subMesh.ibv_);
		// SRVの設定
		cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
		cmdList->SetGraphicsRootDescriptorTable(3, material->GetEnvironmentTextureSRVHandle());
		// ドローコール
		cmdList->DrawIndexedInstanced(UINT(subMesh.indices_.size()), 1, 0, 0, 0);
	}
}

void Renderer::DrawNodeInstance(InstancedModel* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld) {
	Matrix4x4 nodeWorld = parentWorld * node->localMatrix;

	// トランスフォーム更新
	std::vector<Vector4> colors;
	colors.resize(model->GetNumInstance());
	for (auto& color : colors) {
		color = { 1,1,1,1 };
	}
	model->UpdateInstanceTransformWithNode(camera_, nodeWorld, colors);

	// メッシュを描画
	for (uint32_t meshIndex : node->meshIndices) {
		DrawMeshInstance(model, model->GetData()->meshes[meshIndex].get());
	}

	// 子ノード
	for (auto& child : node->children) {
		DrawNodeInstance(model, cmdList, child.get(), nodeWorld);
	}
}

void Renderer::DrawMeshInstance(InstancedModel* model, Mesh* mesh) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();

	// 各メッシュを描画
	for (const auto& subMesh : mesh->GetPrimitives()) {
		Material* material = model->GetMaterial(subMesh.materialIndex_);

		// マテリアル更新
		material->UpdateGPU();

		// マテリアルCBufferの場所を設定
		cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
		// モデル描画
		cmdList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView_);	// VBVを設定
		// SRVの設定
		cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
		cmdList->SetGraphicsRootDescriptorTable(3, material->GetEnvironmentTextureSRVHandle());
		// ドローコール
		cmdList->DrawInstanced(UINT(subMesh.vertices_.size()), model->GetNumInstance(), 0, 0);
	}

}


void Renderer::InitializePlane() {
	auto bufferManager = dxContext_->GetBufferManager();
	UINT sizeInBytes = sizeof(VertexData) * 4;

	// 頂点リソース
	plane_.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	plane_.vbv.BufferLocation = plane_.vertexResource->GetGPUVirtualAddress();
	plane_.vbv.SizeInBytes = sizeInBytes;
	plane_.vbv.StrideInBytes = sizeof(VertexData);

	// 頂点を設定
	VertexData* vertices = nullptr;
	plane_.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	// 右(0,1,2),(2,1,3)
	vertices[0] = { {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[1] = { {-0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[2] = { {0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[3] = { {0.5f, 0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };

	// indexリソース
	plane_.indexResource = bufferManager->CreateUploadBuffer(sizeof(uint32_t) * 6);
	// IBV
	plane_.ibv.BufferLocation = plane_.indexResource->GetGPUVirtualAddress();
	plane_.ibv.SizeInBytes = sizeof(uint32_t) * 6;
	plane_.ibv.Format = DXGI_FORMAT_R32_UINT;
	// データ
	uint32_t* indexData = nullptr;
	plane_.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;	indexData[1] = 1;	indexData[2] = 2; indexData[3] = 1;	indexData[4] = 3;	indexData[5] = 2;
}

void Renderer::InitializeRing() {
	auto bufferManager = dxContext_->GetBufferManager();

	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f; // 外側
	const float kInnerRadius = 0.2f; // 内側
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	UINT sizeInBytes = sizeof(VertexData) * kRingDivide * 4;
	// 頂点リソース
	ring_.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	ring_.vbv.BufferLocation = ring_.vertexResource->GetGPUVirtualAddress();
	ring_.vbv.SizeInBytes = sizeInBytes;
	ring_.vbv.StrideInBytes = sizeof(VertexData);

	VertexData* vertices = nullptr;
	ring_.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));

	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);

		vertices[index * 4 + 0] = { { -sin * kOuterRadius, cos * kOuterRadius, 0, 1}, {u, 0}, {0,0,-1},{1,1,1,1} };
		vertices[index * 4 + 1] = { { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0, 1}, {uNext, 0}, {0,0,-1},{1,1,1,1} };
		vertices[index * 4 + 2] = { { -sin * kInnerRadius, cos * kInnerRadius, 0, 1}, {u, 1}, {0,0,-1},{1,1,1,1} };
		vertices[index * 4 + 3] = { { -sinNext * kInnerRadius, cosNext * kInnerRadius, 0, 1}, {uNext, 1}, {0,0,-1},{1,1,1,1} };
	}

	// indexリソース
	ring_.indexResource = bufferManager->CreateUploadBuffer(sizeof(uint32_t) * 6 * kRingDivide);
	// IBV
	ring_.ibv.BufferLocation = ring_.indexResource->GetGPUVirtualAddress();
	ring_.ibv.SizeInBytes = sizeof(uint32_t) * 6 * kRingDivide;
	ring_.ibv.Format = DXGI_FORMAT_R32_UINT;
	// データ
	uint32_t* indexData = nullptr;
	ring_.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	for (uint32_t index = 0; index < kRingDivide; ++index) {
		uint32_t start = index * 4;
		uint32_t i = index * 6;
		indexData[i + 0] = start + 0;
		indexData[i + 1] = start + 1;
		indexData[i + 2] = start + 2;
		indexData[i + 3] = start + 1;
		indexData[i + 4] = start + 3;
		indexData[i + 5] = start + 2;
	}
}

void Renderer::InitializeLine() {
	auto bufferManager = dxContext_->GetBufferManager();
	int maxLines = 4096;

	UINT sizeInBytes = sizeof(VertexData) * maxLines * 2;
	// 頂点リソース
	line_.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	line_.vbv.BufferLocation = line_.vertexResource->GetGPUVirtualAddress();
	line_.vbv.SizeInBytes = sizeInBytes;
	line_.vbv.StrideInBytes = sizeof(VertexData);

	VertexData* vertices = nullptr;
	line_.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));

}

void Renderer::InitializeSphere() {
	auto bufferManager = dxContext_->GetBufferManager();

	// 分割数
	const uint32_t kSubdivision = 16;
	const float kLonEvery = 2.0f * float(std::numbers::pi) / float(kSubdivision);
	const float kLatEvery = float(std::numbers::pi) / float(kSubdivision);

	UINT sizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	// 頂点リソース
	sphere_.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	sphere_.vbv.BufferLocation = sphere_.vertexResource->GetGPUVirtualAddress();
	sphere_.vbv.SizeInBytes = sizeInBytes;
	sphere_.vbv.StrideInBytes = sizeof(VertexData);

	VertexData* vertices = nullptr;
	sphere_.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));

	// 頂点データの書き込み
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		// 各バンドの南端緯度と北端緯度
		float lat = -0.5f * float(std::numbers::pi) + kLatEvery * float(latIndex);
		float latN = lat + kLatEvery;
		// sin/cos を一度だけ計算
		float cosLat = cos(lat);
		float sinLat = sin(lat);
		float cosLatN = cos(latN);
		float sinLatN = sin(latN);

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = kLonEvery * float(lonIndex);
			float cosLon = cos(lon);
			float sinLon = sin(lon);
			float cosNextLon = cos(lon + kLonEvery);
			float sinNextLon = sin(lon + kLonEvery);

			// テクスチャ座標
			float u = float(lonIndex) / float(kSubdivision);
			float nextU = float(lonIndex + 1) / float(kSubdivision);
			float v = 1.0f - float(latIndex) / float(kSubdivision);
			float nextV = 1.0f - float(latIndex + 1) / float(kSubdivision);

			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 頂点位置
			// BL
			vertices[start].position = { cosLat * cosLon,  sinLat,  cosLat * sinLon, 1.0f };
			vertices[start].texcoord = { u,  v };
			// BR
			vertices[start + 1].position = { cosLat * cosNextLon, sinLat,  cosLat * sinNextLon, 1.0f };
			vertices[start + 1].texcoord = { nextU, v };
			// TL
			vertices[start + 2].position = { cosLatN * cosLon,  sinLatN, cosLatN * sinLon, 1.0f };
			vertices[start + 2].texcoord = { u,  nextV };
			// TR 
			vertices[start + 3].position = { cosLatN * cosNextLon, sinLatN, cosLatN * sinNextLon, 1.0f };
			vertices[start + 3].texcoord = { nextU, nextV };

			// 同じ座標の頂点を代入
			vertices[start + 4] = vertices[start + 2]; // TL
			vertices[start + 5] = vertices[start + 1]; // BR

			// 法線
			for (UINT i = 0; i < 6; ++i) {
				vertices[start + i].normal = Normalize(Vector3{ vertices[start + i].position.x, vertices[start + i].position.y,vertices[start + i].position.z });
			}
		}
	}

	// indexリソース
	sphere_.indexResource = bufferManager->CreateUploadBuffer(sizeof(uint32_t) * kSubdivision * kSubdivision * 6);
	// IBV
	sphere_.ibv.BufferLocation = sphere_.indexResource->GetGPUVirtualAddress();
	sphere_.ibv.SizeInBytes = sizeof(uint32_t) * kSubdivision * kSubdivision * 6;
	sphere_.ibv.Format = DXGI_FORMAT_R32_UINT;
	// データ
	uint32_t* indexData = nullptr;
	sphere_.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {

			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			uint32_t i = start;

			// 1枚の四角形（2三角形）
			indexData[i + 0] = start + 0; // BL
			indexData[i + 1] = start + 1; // BR
			indexData[i + 2] = start + 2; // TL

			indexData[i + 3] = start + 1; // BR
			indexData[i + 4] = start + 3; // TR
			indexData[i + 5] = start + 2; // TL
		}
	}
}

void Renderer::InitializeSkybox() {
	auto bufferManager = dxContext_->GetBufferManager();
	UINT sizeInBytes = sizeof(VertexData) * 24;

	// 頂点リソース
	skybox.shapeData.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	skybox.shapeData.vbv.BufferLocation = skybox.shapeData.vertexResource->GetGPUVirtualAddress();
	skybox.shapeData.vbv.SizeInBytes = sizeInBytes;
	skybox.shapeData.vbv.StrideInBytes = sizeof(VertexData);

	// 頂点を設定
	VertexData* vertices = nullptr;
	skybox.shapeData.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	// 右(0,1,2),(2,1,3)
	vertices[0] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[1] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[2] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[3] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 左(4,5,6),(6,5,7)
	vertices[4] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[5] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[6] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[7] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 前(8,9,10),(10,9,11)
	vertices[8] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[9] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[10] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[11] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 後(12,13,14),(14,13,15)
	vertices[12] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[13] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[14] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[15] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 上(16,17,18),(18,17,19)
	vertices[16] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[17] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[18] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[19] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 下(20,21,22),(22,21,23)
	vertices[20] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[21] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[22] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[23] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };

	// indexリソース
	skybox.shapeData.indexResource = bufferManager->CreateUploadBuffer(sizeof(uint32_t) * 36);
	// IBV
	skybox.shapeData.ibv.BufferLocation = skybox.shapeData.indexResource->GetGPUVirtualAddress();
	skybox.shapeData.ibv.SizeInBytes = sizeof(uint32_t) * 36;
	skybox.shapeData.ibv.Format = DXGI_FORMAT_R32_UINT;
	// データ
	uint32_t* indexData = nullptr;
	skybox.shapeData.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;	indexData[1] = 1;	indexData[2] = 2; indexData[3] = 1;	indexData[4] = 3;	indexData[5] = 2;
	indexData[6] = 4;	indexData[7] = 5;	indexData[8] = 6; indexData[9] = 6;	indexData[10] = 5;	indexData[11] = 7;
	indexData[12] = 8;	indexData[13] = 9;	indexData[14] = 10; indexData[15] = 10;	indexData[16] = 9;	indexData[17] = 11;
	indexData[18] = 12;	indexData[19] = 13;	indexData[20] = 14; indexData[21] = 14;	indexData[22] = 13;	indexData[23] = 15;
	indexData[24] = 16;	indexData[25] = 17;	indexData[26] = 18; indexData[27] = 18;	indexData[28] = 17;	indexData[29] = 19;
	indexData[30] = 20;	indexData[31] = 21;	indexData[32] = 22; indexData[33] = 22;	indexData[34] = 21;	indexData[35] = 23;

	// Material
	skybox.materialResource =
		bufferManager->CreateUploadBuffer(sizeof(MaterialData));

	skybox.materialResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skybox.materialData));

	MaterialData data;
	data.color = { 1,1,1,1 };
	data.uvTransform = MakeIdentity4x4();
	data.enableLighting = false;
	data.useTexture = true;
	*skybox.materialData = data;

	// Transform
	skybox.transformResource =
		bufferManager->CreateUploadBuffer(
			sizeof(TransformationMatrix));

	skybox.transformResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skybox.transformData));

	skybox.transformData->WVP = MakeIdentity4x4();
	skybox.transformData->World = MakeAffineMatrix(Transform({ {1000,1000,1000},{},{} }));
	skybox.transformData->WorldInverseTranspose = MakeIdentity4x4();
}

void Renderer::DrawSkybox(Texture* texture) {
	skybox.transformData->WVP = skybox.transformData->World * camera_->viewMatrix_ * camera_->projectionMatrix_;

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetSkyboxPSO(int(BlendMode::Normal));
	auto rootSig = dxContext_->GetRootSignatureManager()->GetSkyboxRootSignature().Get();

	cmdList->SetPipelineState(pso);
	cmdList->SetGraphicsRootSignature(rootSig);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &skybox.shapeData.vbv);
	cmdList->IASetIndexBuffer(&skybox.shapeData.ibv);
	cmdList->SetGraphicsRootConstantBufferView(0, skybox.materialResource->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, skybox.transformResource->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, texture->GetSRVHandle());
	cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

//void Renderer::DrawLine(const Vector3& start, const Vector3& end) {
//
//}

void Renderer::SetPostEffectType(PostEffectType type) {
	dxContext_->SetPostEffectType(type);
}

void Renderer::SetCamera(Camera* camera) {
	camera_ = camera; 
	dxContext_->SetCamera(camera);
}

void Renderer::BeginFrame() {
	dxContext_->BeginFrame();
}

void Renderer::EndFrame() {
	dxContext_->EndFrame();
}