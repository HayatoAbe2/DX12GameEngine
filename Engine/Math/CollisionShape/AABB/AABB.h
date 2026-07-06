#pragma once
#include "Engine/Math/Vector2/Vector2.h"
#include "Engine/Math/Vector3/Vector3.h"

struct AABB2D {
	Vector2 min;
	Vector3 max;
};

bool CheckCollision(const AABB2D& aabb1, const AABB2D& aabb2);
bool CheckCollision(const AABB2D& aabb, const Vector2& point);


struct AABB3D {	
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

bool CheckCollision(const AABB3D& aabb1, const AABB3D& aabb2);
bool CheckCollision(const AABB3D& aabb, const Vector3& point);