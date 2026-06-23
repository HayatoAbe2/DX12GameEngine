#include "../Fullscreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t mask = gMaskTexture.Sample(gSampler, input.texcoord);
    if (mask <= 0.5f)
    {
        discard;
    }
    float32_t edge = 1.0f - smoothstep(0.5f, 0.53f, mask);
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    output.color.rgb += edge * edge * float32_t3(1.0f, 0.4f, 0.3f);
    output.color.a = 1.0f;
    return output;
}