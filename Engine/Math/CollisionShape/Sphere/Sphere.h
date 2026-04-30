#pragma once
#include "Engine/Math/Vector3/Vector3.h"
class Sphere {
public:
	static bool CheckCollision(const Sphere sphere1, const Sphere sphere2);

	float radius;
	Vector3 center;
};

