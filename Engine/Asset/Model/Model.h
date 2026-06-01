#pragma once
#include "ModelData.h"
#include "Engine/Object/Transform.h"
#include "Engine/Asset/Model/Node.h"
#include "Engine/Asset/Model/Skeleton.h"
#include "Engine/Asset/Model/AnimationPlayer/AnimationPlayer.h"

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>

class Renderer;
class Camera;
class BufferManager;
class Model {
public:

	void Update();

	///
	/// Setter
	///

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetAnimation(std::shared_ptr<Animation> animation) { animationPlayer_ = std::make_unique<AnimationPlayer>(animation, rootNode_->name); }
	
	// データの設定
	void CopyModelData(std::shared_ptr<ModelData> data, BufferManager* bufferManager);

	// 根ノードを設定
	void SetRootNode(std::unique_ptr<ModelNode> rootNode) { rootNode_ = std::move(rootNode); }

	void SetSkeleton(std::shared_ptr<Skeleton> skeleton) { skeleton_ = skeleton; }

	///
	/// Getter
	///

	// トランスフォーム
	Transform GetTransform() { return transform_; }

	// データの取得
	std::shared_ptr<const ModelData> GetData() { return data_; }

	// マテリアル取得
	Material* GetMaterial(int index) { return material_[index].get(); }

	// rootNode取得
	ModelNode* GetRootNode() { return rootNode_.get(); }

private:
	void UpdateSkeleton();

	// アニメーション再生
	std::unique_ptr<AnimationPlayer> animationPlayer_ = nullptr;

	// トランスフォーム
	Transform transform_ = { { 1,1,1 },{}, {} };

	// モデルデータ
	std::shared_ptr<ModelData> data_ = nullptr;

	// マテリアル
	std::vector<std::unique_ptr<Material>> material_{};

	// 根ノード
	std::unique_ptr<ModelNode> rootNode_{};

	// スケルトン
	std::shared_ptr<Skeleton> skeleton_ = nullptr;
};