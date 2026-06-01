#pragma once

struct Vector3;
struct Matrix4x4;

struct Quaternion {
	float x;
	float y;
	float z;
	float w;

	Quaternion& operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}
};

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);
// 単位Quaternion
Quaternion IdentityQuaternion();
// 共役
Quaternion Conjugate(const Quaternion& quaternion);
// ノーム
float Norm(const Quaternion& quaternion);
Quaternion Normalize(const Quaternion& quaternion);
Quaternion Inverse(const Quaternion& quaternion);

// 任意軸回転→Quaternion
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

// ベクトルをQuaternionで回転
Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);

// Quaternion→回転行列
Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

// 球面線形補間
Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);

Quaternion operator+(const Quaternion& q0, const Quaternion& q1);
Quaternion operator*(const Quaternion& q, const float f);
Quaternion operator*(const float f, const Quaternion& q);