#include "Particle.hlsli"
#include "../Particle.hlsli"

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 color : COLOR0;

};

struct PerView {
    float32_t4x4 viewProjection;
    float32_t4x4 billboardMatrix;
};

StructuredBuffer<Particle> gParticles : register(t1);
ConstantBuffer<PerView> gPerview : register(b1);

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
    VertexShaderOutput output;
    Particle particle = gParticles[instanceId];

    // WorldMatrix
    float32_t4x4 worldMatrix = gPerview.billboardMatrix;
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate;
        
    output.position = mul(input.position, mul(worldMatrix, gPerview.viewProjection));
    output.texcoord = input.texcoord;
    output.normal = input.normal;
    output.color = particle.color;

    return output;
}