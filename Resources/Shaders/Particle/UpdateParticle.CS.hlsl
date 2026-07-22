#include "Particle.hlsli"

struct PerFrame {
    float32_t time;
    float32_t deltaTime;
};

ConstantBuffer<PerFrame> gPerFrame : register(b0);
RWStructuredBuffer<Particle> gParticles : register(u0);

[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t i = DTid.x;
    if (i < kMaxParticles) {
        
        if (gParticles[i].color.a != 0) {
            // 移動
            gParticles[i].translate += gParticles[i].velocity * gPerFrame.deltaTime;

			// 時間で透明度変更
            gParticles[i].currentTime += gPerFrame.deltaTime;
            float alpha = 1.0f - (gParticles[i].currentTime / gParticles[i].lifeTime);
            gParticles[i].color.a = saturate(alpha);
        }
    }
}