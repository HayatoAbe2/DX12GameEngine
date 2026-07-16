#include "Particle.hlsli"

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeCounter : register(u1);

// パーティクル初期化
[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {    
    uint32_t index = DTid.x;
    if (index < kMaxParticles) {
        // 未初期化→0にする
        gParticles[index] = (Particle) 0;
    }
    
    if (index == 0) {
        // 1スレッドのみ処理
        gFreeCounter[0] = 0;
    }
}