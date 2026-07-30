#include "Particle.hlsli"

struct PerFrame {
    float32_t time;
    float32_t deltaTime;
};

ConstantBuffer<PerFrame> gPerFrame : register(b0);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t i = DTid.x;
    if (i < kMaxParticles) {
        
        if (gParticles[i].color.a > 0) {
            // 移動
            gParticles[i].translate += gParticles[i].velocity * gPerFrame.deltaTime;

			// 時間で透明度変更
            gParticles[i].currentTime += gPerFrame.deltaTime;
            float alpha = 1.0f - (gParticles[i].currentTime / gParticles[i].lifeTime);
            gParticles[i].color.a = saturate(alpha);
            
             // 色が0になったら解放
            if (gParticles[i].color.a <= 0) {
                gParticles[i].scale = float32_t3(0.0f, 0.0f, 0.0f);

                int32_t freeListIndex;
                InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
            
                if ((freeListIndex + 1) < kMaxParticles) {
                // freeListにこのパーティクルのIndexを戻す
                    gFreeList[freeListIndex + 1] = i;
                } else {
                    InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
                }
            }
        }
    }
}