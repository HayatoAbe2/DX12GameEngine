#pragma once
#include "Engine/Math/MathUtils.h"
#include <cstdint>
#include <Windows.h>

struct MaterialData {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	UINT useTexture;
	float shininess = 32;
	UINT useEnvironmentMap;
	float environmentIntensity;
};