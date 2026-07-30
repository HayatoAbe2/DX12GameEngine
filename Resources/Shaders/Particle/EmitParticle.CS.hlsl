#include "Particle.hlsli"
#include "Random.hlsli"

struct EmitterSphere {
    float32_t3 translate;
    float radius;
    uint32_t count;
    float frequency;
    float frequencyTime;
    uint32_t emit;
};

struct PerFrame {
    float32_t time;
    float32_t deltaTime;
};

ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

// パーティクルEmit
[numthreads(1, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    if (gEmitter.emit != 0) {
        RandomGenerator generator;
        generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
        
        // 出現させる
        for (uint32_t i = 0; i < gEmitter.count; ++i) {
            int32_t freeListIndex;
            // FreeListからIndex取得
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < kMaxParticles) { // 空きあり
                uint32_t particleIndex = gFreeList[freeListIndex];
             
                gParticles[particleIndex].scale = generator.Generate3d();
                gParticles[particleIndex].translate = generator.Generate3d();
                gParticles[particleIndex].color.rgb = generator.Generate3d();
                gParticles[particleIndex].color.a = 1.0f;
                gParticles[particleIndex].velocity = generator.Generate3d() - float32_t3(0.5f, 0.5f, 0.5f);
                gParticles[particleIndex].lifeTime = 3.0f;
                gParticles[particleIndex].currentTime = 0.0f;
            } else {
                // indexを戻す
                InterlockedAdd(gFreeListIndex[0], 1);
                break;
            }
        }
    }
}