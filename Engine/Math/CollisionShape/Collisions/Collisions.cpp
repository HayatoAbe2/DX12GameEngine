#include "Collisions.h"

bool CheckCollision(const AABB3D& aabb, const Ray& ray) {
	float tNear = -FLT_MAX;
	float tFar = FLT_MAX;

	// 0除算回避(軸に並行な場合)
	if (ray.diff.x == 0.0f) {
		if (ray.origin.x < aabb.min.x || ray.origin.x > aabb.max.x) {
			// 当たらない
			return false;
		}

	} else {
		float t1 = (aabb.min.x - ray.origin.x) / ray.diff.x;
		float t2 = (aabb.max.x - ray.origin.x) / ray.diff.x;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}

	if (ray.diff.y == 0.0f) {
		if (ray.origin.y < aabb.min.y || ray.origin.y > aabb.max.y) {
			return false;
		}
	} else {
		float t1 = (aabb.min.y - ray.origin.y) / ray.diff.y;
		float t2 = (aabb.max.y - ray.origin.y) / ray.diff.y;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}
	if (ray.diff.z == 0.0f) {
		if (ray.origin.z < aabb.min.z || ray.origin.z > aabb.max.z) {
			return false;
		}
	} else {
		float t1 = (aabb.min.z - ray.origin.z) / ray.diff.z;
		float t2 = (aabb.max.z - ray.origin.z) / ray.diff.z;

		if (t1 > t2) std::swap(t1, t2);

		tNear = (std::max)(tNear, t1);
		tFar = (std::min)(tFar, t2);

		if (tNear > tFar) {
			return false;
		}
	}

	return tNear <= tFar && tFar >= 0.0f;
}

bool CheckCollision(const Ray& ray, const AABB3D& aabb) {
	return CheckCollision(aabb, ray);
}
