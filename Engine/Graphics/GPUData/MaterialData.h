#pragma once
#include "Engine/Math/MathUtils.h"
#include <cstdint>
#include <Windows.h>

struct MaterialData {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess = 32;
	UINT useTexture;
	float padding2[2];
};