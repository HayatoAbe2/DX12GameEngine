#include "Particle.hlsli"

static const uint32_t kMaxParticles = 1024;
RWStructuredBuffer<Particle> gParticles : register(u0);

// パーティクル初期化
[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t index = DTid.x;
    if (index < kMaxParticles) {
        // 未初期化→0にする
        gParticles[index] = (Particle) 0;
        
        
        
        gParticles[index].scale = float32_t3(10.0f, 10.0f, 10.0f);
        gParticles[index].translate = float32_t3(3.0f, 3.0f, 3.0f);
        gParticles[index].color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}