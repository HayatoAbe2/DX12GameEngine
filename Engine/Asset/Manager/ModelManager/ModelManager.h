#pragma once
#include "Engine/SceneObject/Model/Model.h"
#include "Engine/SceneObject/Model/InstancedModel.h"
#include "Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h"
#include "Engine/SceneObject/Primitive/Primitive.h"

#include <unordered_map>
#include <string>
#include <memory>

#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

class ModelData;
struct ModelNode;
struct SubMeshRuntime;

class DirectXContext;
class Logger;
class CommandListManager;
class DescriptorHeapManager;
class SRVManager;
class BufferManager;
class ConstantBufferManager;
class TextureManager;

class ModelManager {
public:
	ModelManager(DirectXContext* dxContext, Logger* logger, TextureManager* textureManager);
	std::unique_ptr<Model> Load(uint32_t id, uint32_t textureId, uint32_t envTextureId, uint32_t materialId, const std::string& directoryPath, const std::string& filename);
	std::unique_ptr<InstancedModel> Load(uint32_t id, uint32_t textureId, uint32_t envTextureId, uint32_t materialId, const std::string& directoryPath, const std::string& filename, const int numInstance_);
	std::unique_ptr<ParticleSystem> CreateParticleInstanceResource(int numInstance, uint32_t id);
	std::unique_ptr<Primitive> CreatePrimitive(uint32_t id);
	std::unique_ptr<Material> LoadMaterial(std::shared_ptr<Texture> texture, uint32_t id, uint32_t textureId);

private:
	// 関数内で使う関数
	std::unique_ptr<ModelNode> ReadNode(aiNode* node);
	SubMeshRuntime CreateSubMesh(aiMesh* aiMesh);
	SubMeshData CreateSubMeshData(aiMesh* aiMesh);
	void CreateInstancingSRV(InstancedModel* model, const int numInstance_);
	Matrix4x4 ConvertAssimpMatrixToLHRow(const aiMatrix4x4& m);
	Skeleton CreateSkeleton(const ModelNode& rootNode);
	int32_t CreateJoint(const ModelNode & node, const std::optional<int32_t>&parent, std::vector<Joint>&joints);
	SkinClusterRuntime CreateSkinCluster(const Skeleton& skeleton, ModelData* data);

	// デバイス
	ID3D12Device* device_ = nullptr;
	// ログ出力クラス
	Logger* logger_ = nullptr;
	// CommandList管理クラス
	CommandListManager* commandListManager_ = nullptr;
	// DescriptorHeap管理クラス
	DescriptorHeapManager* descriptorHeapManager_ = nullptr;
	// SRV管理クラス
	SRVManager* srvManager_ = nullptr;
	// バッファ管理クラス
	BufferManager* bufferManager_ = nullptr;
	// CB管理クラス
	ConstantBufferManager* cbManager_ = nullptr;
	// テクスチャ管理クラス
	TextureManager* textureManager_ = nullptr;

	// モデルデータのキャッシュ
	std::unordered_map<std::string, std::shared_ptr<ModelData>> modelDataCache_;
};

