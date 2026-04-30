#pragma once
#include "Engine/Object/Transform.h"
#include "Engine/Math/MathUtils.h"

struct Particle {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    int lifeTime;
    bool alive = false;
};