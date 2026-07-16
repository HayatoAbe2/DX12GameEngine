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

struct PerView {
    Matrix4x4 viewProjection;
    Matrix4x4 billboardMatrix;
};

struct PerFrame {
    float time;
    float deltaTime;
};

struct EmitterSphere {
    Vector3 translate;
    float radius;
    uint32_t count;
    float frequency;
    float frequencyTime;
    uint32_t emit;
};