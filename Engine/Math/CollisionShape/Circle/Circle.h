#pragma once
#include "Engine/Math/Vector2/Vector2.h"

struct Circle {
    float radius;
    Vector2 center;

    static bool CheckCollision(const Circle& a, const Circle& b) {
        Vector2 diff = a.center - b.center;
        float distSq = Dot(diff, diff);
        float radiusSum = a.radius + b.radius;

        return distSq <= radiusSum * radiusSum;
    }
};