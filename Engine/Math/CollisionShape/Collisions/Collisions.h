#pragma once
#include "Engine/Math/CollisionShape/AABB/AABB.h"
#include "Engine/Math/CollisionShape/Ray/Ray.h"
#include "Engine/Math/CollisionShape/Circle/Circle.h"
#include "Engine/Math/CollisionShape/Sphere/Sphere.h"
#include <algorithm>

// Collisionまとめ+図形どうしの判定

// AABBと半直線の衝突判定
bool CheckCollision(const AABB3D& aabb, const Ray& ray);
bool CheckCollision(const Ray& ray, const AABB3D& aabb);