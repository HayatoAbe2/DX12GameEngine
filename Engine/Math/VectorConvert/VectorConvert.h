#pragma once
#include "Engine/Math/Vector2/Vector2.h"
#include "Engine/Math/Vector3/Vector3.h"
#include "Engine/Math/Vector4/Vector4.h"

Vector2 ToXZ(const Vector3& v);
Vector2 ToXY(const Vector3& v);
Vector3 ToXYZ(const Vector4& v);