#pragma once
#include "Engine/Asset/Animation.h"
#include "Engine/Asset/Model/Node.h"
#include "Engine/Asset/Model/Skeleton.h"
#include <string>
#include <memory>

class AnimationPlayer {
public:
	AnimationPlayer(std::shared_ptr<Animation> animation, const std::string& rootNodeName);
	
	// ノードアニメーション
	void Update(ModelNode& rootNode);

	// ボーンアニメーション
	void Update(Skeleton& skeleton);

private:
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	float animationTime_ = 0.0f;
	
	NodeAnimation nodeAnim_;
	std::shared_ptr<Animation> animation_ = nullptr;
};

