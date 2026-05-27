#include "AnimationPlayer.h"
#include <cmath>

AnimationPlayer::AnimationPlayer(std::unique_ptr<Animation> animation, const std::string& rootNodeName) {
	animation_ = std::move(animation);
	nodeAnim_ = animation_->nodeAnimations[rootNodeName];
}

void AnimationPlayer::Update(ModelNode& rootNode) {
	animationTime_ += 1.0f / 60.0f;
	animationTime_ = std::fmod(animationTime_, animation_->duration);

	Vector3 translate = CalculateValue(nodeAnim_.translate, animationTime_);
	Quaternion rotate = Normalize(CalculateValue(nodeAnim_.rotate, animationTime_));
	Vector3 scale = CalculateValue(nodeAnim_.scale, animationTime_);

	rootNode.localMatrix = MakeAffineMatrix(scale, rotate, translate);
}

Vector3 AnimationPlayer::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	assert(!keyframes.empty()); // キーフレームなし
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーフレーム一つor最初のキーフレーム前
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		// キーフレーム間に現在時間があれば補間
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	// 最後のキーフレーム
	return (*keyframes.rbegin()).value;
}

Quaternion AnimationPlayer::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	assert(!keyframes.empty()); // キーフレームなし
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーフレーム一つor最初のキーフレーム前
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		// キーフレーム間に現在時間があれば補間
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	// 最後のキーフレーム
	return (*keyframes.rbegin()).value;
}