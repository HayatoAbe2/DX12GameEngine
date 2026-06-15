#pragma once
#include "Engine/Math/Vector2/Vector2.h"

struct Circle {
    float radius;
    Vector2 center;

    bool CheckCollision(const Circle& other) {
        Vector2 diff = center - other.center;
        float distSq = Dot(diff, diff);
        float radiusSum = radius + other.radius;

        return distSq <= radiusSum * radiusSum;
    }
};