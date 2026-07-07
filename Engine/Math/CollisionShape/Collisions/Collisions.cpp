#include "Collisions.h"

bool CheckCollision(const AABB2D& aabb1, const AABB2D& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y)) {
		return true;
	}
	return false;
}

bool CheckCollision(const AABB2D& aabb, const Vector2& point) {
	if (aabb.min.x <= point.x && aabb.max.x >= point.x &&
		aabb.min.y <= point.y && aabb.max.y >= point.y) {
		return true;
	}
	return false;
}
bool CheckCollision(const Vector2& point, const AABB2D& aabb) {
	return CheckCollision(aabb, point);
}

bool CheckCollision(const AABB3D& aabb1, const AABB3D& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) { // z軸
		return true;
	}
	return false;
}

bool CheckCollision(const AABB3D& aabb, const Vector3& point) {
	if (aabb.min.x <= point.x && aabb.max.x >= point.x &&
		aabb.min.y <= point.y && aabb.max.y >= point.y &&
		aabb.min.z <= point.z && aabb.max.z >= point.z) {
		return true;
	}
	return false;
}
bool CheckCollision(const Vector3& point, const AABB3D& aabb) {
	return CheckCollision(aabb, point);
}

bool CheckCollision(const AABB3D& aabb, const Ray3D& ray) {
	float tNear = -FLT_MAX;
	float tFar = FLT_MAX;

	// 0除算回避(軸に並行な場合)
	if (ray.direction.x == 0.0f) {
		if (ray.origin.x < aabb.min.x || ray.origin.x > aabb.max.x) {
			// 当たらない
			return false;
		}

	} else {
		float t1 = (aabb.min.x - ray.origin.x) / ray.direction.x;
		float t2 = (aabb.max.x - ray.origin.x) / ray.direction.x;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}

	if (ray.direction.y == 0.0f) {
		if (ray.origin.y < aabb.min.y || ray.origin.y > aabb.max.y) {
			return false;
		}
	} else {
		float t1 = (aabb.min.y - ray.origin.y) / ray.direction.y;
		float t2 = (aabb.max.y - ray.origin.y) / ray.direction.y;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}
	if (ray.direction.z == 0.0f) {
		if (ray.origin.z < aabb.min.z || ray.origin.z > aabb.max.z) {
			return false;
		}
	} else {
		float t1 = (aabb.min.z - ray.origin.z) / ray.direction.z;
		float t2 = (aabb.max.z - ray.origin.z) / ray.direction.z;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}

	return tNear <= tFar && tFar >= 0.0f;
}
bool CheckCollision(const Ray3D& ray, const AABB3D& aabb) {
	return CheckCollision(aabb, ray);
}

bool CheckCollision(const Sphere& a, const Sphere& b) {
	return Length(a.center - b.center) <= (a.radius + b.radius);
}

bool CheckCollision(const Circle& a, const Circle& b) {
	Vector2 diff = a.center - b.center;
	float distSq = Dot(diff, diff);
	float radiusSum = a.radius + b.radius;

	return distSq <= radiusSum * radiusSum;
}

bool CheckCollision(const Segment2D& segment, const Circle& circle) {
	Vector2 ab = segment.end - segment.start;
	Vector2 ac = circle.center - segment.start;

	// 射影
	if (Dot(ab, ab) == 0) return Length(circle.center - segment.start) <= circle.radius; // 0除算対策
	float t = Dot(ac, ab) / Dot(ab, ab);

	// 線分の範囲内に入れる
	t = std::clamp(t, 0.0f, 1.0f);

	// 最近接点
	Vector2 nearest = segment.start + ab * t;

	// 距離
	Vector2 diff = circle.center - nearest;
	return Dot(diff, diff) <= circle.radius * circle.radius;
}
bool CheckCollision(const Circle& circle, const Segment2D& segment) {
	return CheckCollision(segment, circle);
}

bool CheckCollision(const Segment3D& segment, const Sphere& sphere) {
	Vector3 ab = segment.end - segment.start;
	Vector3 ac = sphere.center - segment.start;

	// 射影
	if (Dot(ab, ab) == 0) return Length(sphere.center - segment.start) <= sphere.radius; // 0除算対策
	float t = Dot(ac, ab) / Dot(ab, ab);

	// 線分の範囲内に入れる
	t = std::clamp(t, 0.0f, 1.0f);

	// 最近接点
	Vector3 nearest = segment.start + ab * t;

	// 距離
	Vector3 diff = sphere.center - nearest;
	return Dot(diff, diff) <= sphere.radius * sphere.radius;
}
bool CheckCollision(const Sphere& sphere, const Segment3D& segment) {
	return CheckCollision(segment, sphere);
}

bool CheckCollision(const Capsule2D& capsule, const Circle& circle) {
	Circle c = circle;
	c.radius += capsule.radius;

	return CheckCollision(capsule.segment, c);
}
bool CheckCollision(const Circle& circle, const Capsule2D& capsule) {
	return CheckCollision(capsule, circle);
}

bool CheckCollision(const Capsule3D& capsule, const Sphere& sphere) {
	Sphere s = sphere;
	s.radius += capsule.radius;

	return CheckCollision(capsule.segment, s);
}
bool CheckCollision(const Sphere& sphere, const Capsule3D& capsule) {
	return CheckCollision(capsule, sphere);
}