#pragma once
#include "Engine/Math/CollisionShape/AABB.h"
#include "Engine/Math/CollisionShape/Capsule.h"
#include "Engine/Math/CollisionShape/Circle.h"
#include "Engine/Math/CollisionShape/Ray.h"
#include "Engine/Math/CollisionShape/Segment.h"
#include "Engine/Math/CollisionShape/Sphere.h"
#include <algorithm>

// Collisionまとめ

// AABB同士
bool CheckCollision(const AABB2D& aabb1, const AABB2D& aabb2);
bool CheckCollision(const AABB3D& aabb1, const AABB3D& aabb2);

// AABBと2D点
bool CheckCollision(const AABB2D& aabb, const Vector2& point);
bool CheckCollision(const Vector2& point, const AABB2D& aabb);

// AABBと3D点
bool CheckCollision(const AABB3D& aabb, const Vector3& point);
bool CheckCollision(const Vector3& point, const AABB3D& aabb);

// AABBと半直線の衝突判定
bool CheckCollision(const AABB3D& aabb, const Ray3D& ray);
bool CheckCollision(const Ray3D& ray, const AABB3D& aabb);

// Circle同士
bool CheckCollision(const Circle& a, const Circle& b);

// Sphere同士
bool CheckCollision(const Sphere& a, const Sphere& b);

// 線分と円
bool CheckCollision(const Segment2D& segment, const Circle& circle);
bool CheckCollision(const Circle& circle, const Segment2D& segment);

// 線分と球
bool CheckCollision(const Segment3D& segment, const Sphere& sphere);
bool CheckCollision(const Sphere& sphere, const Segment3D& segment);