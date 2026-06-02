#pragma once
#include "Engine/Math/Vector3/Vector3.h"

struct Sphere {
    float radius;
    Vector3 center;

    static bool CheckCollision(const Sphere& a, const Sphere& b) {
        return Length(a.center - b.center) <= (a.radius + b.radius);
    }
};