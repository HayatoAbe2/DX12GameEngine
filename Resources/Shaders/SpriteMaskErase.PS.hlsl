#include "Object3d.hlsli"

struct Material {
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    int32_t useTexture;
    float32_t shininess;
    int32_t useEnvironmentMap;
    float32_t environmentIntensity;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

// スプライトの下にあるアウトライン用マスクを消す
// rgb=0、α=スプライトの不透明度で出力し、アルファブレンドで「マスク × (1 - α)」にする
// (描画される範囲は、通常のスプライト描画(Object3d.PS.hlsl)と同じになるようにdiscardも揃えている)
PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 baseColor;
    if (gMaterial.useTexture != 0) {
        float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

        if (textureColor.a < 0.1f) {
            discard;
        }

        baseColor = gMaterial.color * textureColor;
    } else {
        baseColor = gMaterial.color;
    }

    // 頂点カラー
    baseColor *= input.color;

    if (baseColor.a == 0.0f) {
        discard;
    }

    output.color = float32_t4(0.0f, 0.0f, 0.0f, baseColor.a);
    return output;
}
