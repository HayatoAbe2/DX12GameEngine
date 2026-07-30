#include "Particle.hlsli"

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

// パーティクル初期化
[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {    
    uint32_t index = DTid.x;
    if (index < kMaxParticles) {
        // 未初期化→0にする
        gParticles[index] = (Particle) 0;
        // 空きリスト初期化
        gFreeList[index] = index;
    }
    
    // 1スレッドのみ処理
    if (index == 0) {
        // 末尾
        gFreeListIndex[0] = kMaxParticles - 1;
    }
}