#include "Quaternion.h"
#include "Engine/Math/Matrix4x4/Matrix4x4.h"
#include "Engine/Math/Vector3/Vector3.h"
#include <cmath>
#include <algorithm>

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	return Quaternion{
		lhs.y * rhs.z - lhs.z * rhs.y + rhs.w * lhs.x + lhs.w * rhs.x,
		lhs.z * rhs.x - lhs.x * rhs.z + rhs.w * lhs.y + lhs.w * rhs.y,
		lhs.x * rhs.y - lhs.y * rhs.x + rhs.w * lhs.z + lhs.w * rhs.z,
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
	};
}

Quaternion IdentityQuaternion() {
	return Quaternion{ 0,0,0,1 };
}

Quaternion Conjugate(const Quaternion& quaternion) {
	return Quaternion(
		-quaternion.x,
		-quaternion.y,
		-quaternion.z,
		quaternion.w
	);
}

float Norm(const Quaternion& quaternion) {
	return sqrtf(
		quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w
	);
}

Quaternion Normalize(const Quaternion& quaternion) {
	// 長さ
	float norm = Norm(quaternion);

	if (norm != 0.0f) // ゼロ除算対策
	{
		return Quaternion{
			quaternion.x / norm,
			quaternion.y / norm,
			quaternion.z / norm,
			quaternion.w / norm,
		};
	}

	return quaternion;
}

Quaternion Inverse(const Quaternion& quaternion) {
	Quaternion conj = Conjugate(quaternion);
	float norm = Norm(quaternion);
	return Quaternion{
		conj.x / (norm * norm),
		conj.y / (norm * norm),
		conj.z / (norm * norm),
		conj.w / (norm * norm),
	};
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	return Quaternion{
		axis.x * sinf(angle / 2.0f),
		axis.y * sinf(angle / 2.0f),
		axis.z * sinf(angle / 2.0f),
		cosf(angle / 2.0f)
	};
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	Quaternion vec{ vector.x, vector.y, vector.z, 0 };
	Quaternion normalized = Normalize(quaternion);
	Quaternion result = Multiply(Multiply(normalized, vec), Inverse(normalized));

	return Vector3{ result.x, result.y, result.z };
}

Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
	return Matrix4x4{
		quaternion.w * quaternion.w + quaternion.x * quaternion.x - quaternion.y * quaternion.y - quaternion.z * quaternion.z,
		2.0f * (quaternion.x * quaternion.y + quaternion.w * quaternion.z),
		2.0f * (quaternion.x * quaternion.z - quaternion.w * quaternion.y),
		0,
		2.0f * (quaternion.x * quaternion.y - quaternion.w * quaternion.z),
		quaternion.w * quaternion.w - quaternion.x * quaternion.x + quaternion.y * quaternion.y - quaternion.z * quaternion.z,
		2.0f * (quaternion.y * quaternion.z + quaternion.w * quaternion.x),
		0,
		2.0f * (quaternion.x * quaternion.z + quaternion.w * quaternion.y),
		2.0f * (quaternion.y * quaternion.z - quaternion.w * quaternion.x),
		quaternion.w * quaternion.w - quaternion.x * quaternion.x - quaternion.y * quaternion.y + quaternion.z * quaternion.z,
		0,
		0,
		0,
		0,
		1
	};
}

float Dot(const Quaternion& q0, const Quaternion& q1) {
	return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
	Quaternion to = q1;

	float dot = Dot(q0, q1);

	if (dot < 0.0f) {
		to = -to;
		dot = -dot;
	}

	dot = std::clamp(dot, -1.0f, 1.0f);

	if (dot > 0.9995f) {
		return Normalize(Lerp(q0, to, t));
	}

	float theta = acosf(dot);
	float sinTheta = sinf(theta);

	float scale0 = sinf((1.0f - t) * theta) / sinTheta;
	float scale1 = sinf(t * theta) / sinTheta;

	return Normalize(scale0 * q0 + scale1 * to);
}

Quaternion Lerp(const Quaternion& q0, const Quaternion& q1, float t) {
	return Normalize(q0 * (1.0f - t) + q1 * t);
}

Quaternion operator+(const Quaternion& q0, const Quaternion& q1) {
	return Quaternion{ q0.x + q1.x,q0.y + q1.y,q0.z + q1.z,q0.w + q1.w };
}
Quaternion operator*(const Quaternion& q, const float f) {
	return Quaternion{ q.x * f, q.y * f, q.z * f, q.w * f };
}
Quaternion operator*(const float f, const Quaternion& q) {
	return operator*(q, f);
}
Quaternion operator-(const Quaternion& q) {
	return { -q.x, -q.y, -q.z, -q.w };
}