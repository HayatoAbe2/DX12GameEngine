#pragma once
#include "Engine/Object/Transform.h"
#include <string>
#include <vector>
#include <optional>
#include <Map>

struct Joint {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix; // skeletonSpaceでの変換行列
	std::string name;
	std::vector<int32_t> children;
	int32_t index;
	std::optional<int32_t> parent;
};

struct Skeleton {
	int32_t root;
	std::map<std::string, int32_t> jointMap;
	std::vector<Joint> joints; // 所属しているジョイント
};