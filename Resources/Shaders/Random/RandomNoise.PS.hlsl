#include "../Fullscreen.hlsli"

struct Material
{
    float32_t time;
};
ConstantBuffer<Material> gMaterial : register(b0);

float rand2dTo1d(float32_t2 value)
{
    return frac(sin(dot(value, float2(12.9898, 78.233))) * 43758.5453123);
}

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t random = rand2dTo1d(input.texcoord * gMaterial.time);
    output.color = float32_t4(random, random, random, 1.0f);
    return output;
}