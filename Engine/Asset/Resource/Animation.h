#pragma once
#include <map>
#include <string>
#include <vector>
#include "Engine/Asset/Resource/Resource.h"
#include "Engine/Math/Mathutils.h"

template <typename T>

struct Keyframe {
	T value;
	float time;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

struct NodeAnimation {
	std::vector<KeyframeVector3> translate;
	std::vector<KeyframeQuaternion> rotate;
	std::vector<KeyframeVector3> scale;
};

struct Animation : public Resource{
	Animation(uint32_t id) : Resource(id) {};

	float duration; // 元の全体時間(秒)
	float speed = 1.0f; // 再生速度(1で等倍)
	std::map<std::string, NodeAnimation> nodeAnimations;
	bool isLoop = false;

	// 全体の再生時間が totalTime 秒になるように再生速度を変える(0以下なら等倍に戻す)
	void SetTotalDuration(float totalTime) {
		speed = (totalTime > 0.0f && duration > 0.0f) ? duration / totalTime : 1.0f;
	}

	// 今の再生速度で最初から最後まで再生したときの時間(秒)
	float GetPlayDuration() const {
		return speed > 0.0f ? duration / speed : duration;
	}
};