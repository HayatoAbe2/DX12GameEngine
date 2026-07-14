#pragma once
#include "Engine/Math/Vector2/Vector2.h"
#include "Engine/Math/Vector3/Vector3.h"

struct AABB2D {
	Vector2 min;
	Vector2 max;
};

struct AABB3D {	
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};