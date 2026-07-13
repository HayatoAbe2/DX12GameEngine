#pragma once
#include "Engine/SceneObject/Transform.h"
#include "Engine/Math/MathUtils.h"

struct Particle {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    int lifeTime;
    bool alive = false;
};

struct GPUParticle {
    Vector3 scale;
    Vector3 translate;
    float lifeTime;
    Vector3 velocity;
    float currentTime;
    Vector4 color;
};