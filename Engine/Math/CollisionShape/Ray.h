#pragma once
#include "Engine/Math/Vector2/Vector2.h"
#include "Engine/Math/Vector3/Vector3.h"

struct Ray2D {
	Vector2 origin;
	Vector2 direction;
};

struct Ray3D {
	Vector3 origin;
	Vector3 direction;
};