#pragma once
#include "Engine/Math/Vector3/Vector3.h"
#include "Engine/Math/Quaternion/Quaternion.h"
struct Transform {
	Vector3 scale = {1.0f,1.0f,1.0f};
	Vector3 rotate = {};
	Vector3 translate = {};
};

struct QuaternionTransform {
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Quaternion rotate = IdentityQuaternion();
	Vector3 translate = {};
};