#pragma once
#include "Engine/Asset/Resource/Animation.h"
#include "Engine/SceneObject/Model/Node.h"
#include "Engine/SceneObject/Model/Skeleton.h"
#include <string>
#include <memory>

class AnimationPlayer {
public:
	AnimationPlayer(std::shared_ptr<Animation> animation, const std::string& rootNodeName);
	
	// ノードアニメーション
	void Update(ModelNode& rootNode);

	// ボーンアニメーション
	void Update(Skeleton& skeleton);

	void ChangeAnimation(std::shared_ptr<Animation> animation, float duration);

private:
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	float animationTime_ = 0.0f;
	
	NodeAnimation nodeAnim_;
	std::shared_ptr<Animation> animation_ = nullptr;
	std::shared_ptr<Animation> nextAnimation_ = nullptr;

	float nextAnimationTime_ = 0;
	float blendTime_ = 0;
	float blendDuration_ = 0;
};

