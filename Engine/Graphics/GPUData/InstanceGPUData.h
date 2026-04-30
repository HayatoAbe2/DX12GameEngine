#pragma once
#include "Engine/Math/MathUtils.h"

struct InstanceGPUData {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
	Vector4 Color;
};