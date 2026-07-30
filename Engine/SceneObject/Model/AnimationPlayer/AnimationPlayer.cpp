#include "AnimationPlayer.h"
#include <cmath>
#include "Engine/Contexts/GameContext/GameContext.h"

AnimationPlayer::AnimationPlayer(std::shared_ptr<Animation> animation, const std::string& rootNodeName) {
	animation_ = animation;
	nodeAnim_ = animation_->nodeAnimations[rootNodeName];
}

void AnimationPlayer::Update(ModelNode& rootNode) {
	animationTime_ += GameContext::GetInstance().GetDeltatime();
	animationTime_ = std::fmod(animationTime_, animation_->duration);

	Vector3 translate = CalculateValue(nodeAnim_.translate, animationTime_);
	Quaternion rotate = Normalize(CalculateValue(nodeAnim_.rotate, animationTime_));
	Vector3 scale = CalculateValue(nodeAnim_.scale, animationTime_);

	rootNode.localMatrix = MakeAffineMatrix(scale, rotate, translate);
}

void AnimationPlayer::Update(Skeleton& skeleton) {
	float deltaTime = GameContext::GetInstance().GetDeltatime();
	animationTime_ += deltaTime;
	animationTime_ = std::fmod(animationTime_, animation_->duration);

	// 次のアニメーションへの補間
	if (nextAnimation_) {
		nextAnimationTime_ += deltaTime;
		nextAnimationTime_ = std::fmod(nextAnimationTime_, nextAnimation_->duration);
		blendTime_ += deltaTime;
	}

	float t = 0;
	if (nextAnimation_) {
		t = (std::min)(blendTime_ / blendDuration_, 1.0f);
	}

	for (Joint& joint : skeleton.joints) {
		auto currentIt = animation_->nodeAnimations.find(joint.name);
		// 現在アニメーションに存在しないJointは触らない 
		if (currentIt == animation_->nodeAnimations.end()) {
			continue;
		}

		// jointのTransform
		const NodeAnimation& currentNode = currentIt->second;

		if (nextAnimation_) {
			Vector3 currentTranslate = joint.transform.translate;
			Quaternion currentRotate = joint.transform.rotate;
			Vector3 currentScale = joint.transform.scale;

			if (!currentNode.translate.empty()) {
				currentTranslate = CalculateValue(currentNode.translate, animationTime_);
			}
			if (!currentNode.rotate.empty()) {
				currentRotate = Normalize(CalculateValue(currentNode.rotate, animationTime_));
			}
			if (!currentNode.scale.empty()) {
				currentScale = CalculateValue(currentNode.scale, animationTime_);
			}

			// 次値（存在しなければ現在値を維持）
			Vector3 nextTranslate = currentTranslate;
			Quaternion nextRotate = currentRotate;
			Vector3 nextScale = currentScale;

			if (auto nextIt = nextAnimation_->nodeAnimations.find(joint.name);
				nextIt != nextAnimation_->nodeAnimations.end()) {
				const NodeAnimation& nextNode = nextIt->second;
				if (!nextNode.translate.empty()) {
					nextTranslate = CalculateValue(nextNode.translate, nextAnimationTime_);
				}
				if (!nextNode.rotate.empty()) {
					nextRotate = Normalize(CalculateValue(nextNode.rotate, nextAnimationTime_));
				}
				if (!nextNode.scale.empty()) {
					nextScale = CalculateValue(nextNode.scale, nextAnimationTime_);
				}
			}

			// 補間
			joint.transform.translate = Lerp(currentTranslate, nextTranslate, t);
			joint.transform.rotate = Normalize(Slerp(currentRotate, nextRotate, t));
			joint.transform.scale = Lerp(currentScale, nextScale, t);

		} else {
			if (!currentNode.translate.empty()) {
				joint.transform.translate = CalculateValue(currentNode.translate, animationTime_);
			}
			if (!currentNode.rotate.empty()) {
				joint.transform.rotate = Normalize(CalculateValue(currentNode.rotate, animationTime_));
			}
			if (!currentNode.scale.empty()) {
				joint.transform.scale = CalculateValue(currentNode.scale, animationTime_);
			}
		}

		joint.localMatrix = MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);
	}

	if (t >= 1.0f) {
		// 完全に移行
		animation_ = nextAnimation_;
		animationTime_ = nextAnimationTime_;
		nextAnimation_.reset();
	}
}

void AnimationPlayer::ChangeAnimation(std::shared_ptr<Animation> animation, float duration) {
	// 再生中、切替中のものと同じだったら無視
	if (animation_ == animation || nextAnimation_ == animation) {
		return;
	}

	nextAnimation_ = animation;
	nextAnimationTime_ = 0;
	blendTime_ = 0;
	blendDuration_ = duration;
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