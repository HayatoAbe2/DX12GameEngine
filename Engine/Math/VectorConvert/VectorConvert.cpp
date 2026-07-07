#include "VectorConvert.h"

Vector2 ToXZ(const Vector3& v) {
	return Vector2(v.x, v.z);
}

Vector2 ToXY(const Vector3& v) {
	return Vector2(v.x, v.y);
}

Vector3 ToXYZ(const Vector4& v) {
	return Vector3(v.x, v.y, v.z);
}
