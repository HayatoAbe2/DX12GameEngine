#pragma once
#include "ModelData.h"
#include "Engine/SceneObject/Transform.h"
#include "Engine/SceneObject/Model/Node.h"
#include "Engine/SceneObject/Model/Skeleton.h"
#include "Engine/SceneObject/Model/AnimationPlayer/AnimationPlayer.h"
#include "Engine/SceneObject/Model/Mesh.h"
#include "Engine/SceneObject/SceneObject.h"
#include <vector>
#include <string>
#include <span>

class Renderer;
class Camera;
class BufferManager;
class Model : public SceneObject {
public:
	Model(uint32_t id) : SceneObject(id) {};
	void Update();

	///
	/// Setter
	///

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetAnimation(std::shared_ptr<Animation> animation) { animationPlayer_ = std::make_unique<AnimationPlayer>(animation, rootNode_->name); }

	
	void CopyModelData(std::shared_ptr<ModelData> data, BufferManager* bufferManager);
	void SetRootNode(std::unique_ptr<ModelNode> rootNode) { rootNode_ = std::move(rootNode); } 
	void SetMeshes(std::vector<MeshRuntime> meshes) { meshes_ = meshes; }
	void SetSkeleton(const Skeleton& skeleton) { skeleton_ = skeleton; }
	void SetSkinCluster(const SkinClusterRuntime& skinCluster) { skinCluster_ = skinCluster; }
	void SetDirectoryPath(std::string directoryPath) {directory_ = directoryPath; }

	///
	/// Getter
	///

	// トランスフォーム
	const Transform& GetTransform() { return transform_; }
	// マテリアル取得
	Material* GetMaterial(int index) { return materials_[index].get(); }
	std::vector<Material*> GetMaterials() {
		std::vector<Material*> result;
		result.reserve(materials_.size());

		for (auto& material : materials_) {
			result.push_back(material.get());
		}

		return result;
	}


	std::shared_ptr<const ModelData> GetData() { return data_; }
	const SkinClusterRuntime& GetSkinCluster() { return skinCluster_; }
	std::vector<MeshRuntime>& GetMesh() { return meshes_; }
	const std::vector<MeshRuntime>& GetMesh() const { return meshes_; }
	ModelNode* GetRootNode() { return rootNode_.get(); }
	const std::string& GetDirectoryPath() { return directory_; }

	bool IsUseAnimation() { return animationPlayer_ != nullptr; }
private:
	void UpdateSkeleton();
	void UpdateSkinCluster();

	// トランスフォーム
	Transform transform_;

	// メッシュ
	std::vector<MeshRuntime> meshes_;

	// マテリアル
	std::vector<std::unique_ptr<Material>> materials_;

	// 根ノード
	std::unique_ptr<ModelNode> rootNode_;

	// スケルトン
	Skeleton skeleton_;

	// SkinCluster
	SkinClusterRuntime skinCluster_;

	// アニメーション再生
	std::unique_ptr<AnimationPlayer> animationPlayer_ = nullptr;

	// 共有・生成時で固定データ
	std::shared_ptr<ModelData> data_ = nullptr;

	std::string directory_;
};