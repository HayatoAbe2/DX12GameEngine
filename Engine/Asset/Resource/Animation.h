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

	float duration; // 全体時間
	std::map<std::string, NodeAnimation> nodeAnimations;
};