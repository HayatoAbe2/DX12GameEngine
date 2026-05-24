#pragma once
#include "Engine/Asset/Animation.h"
#include "Engine/Asset/Model/Node.h"
#include <string>
#include <memory>

class AnimationPlayer {
public:
	AnimationPlayer(std::unique_ptr<Animation> animation, const std::string& rootNodeName);
	void Update(ModelNode& rootNode);

private:
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	float animationTime_ = 0.0f;
	
	NodeAnimation nodeAnim_;
	std::unique_ptr<Animation> animation_ = nullptr;
};

