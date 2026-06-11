#include "../Fullscreen.hlsli"

struct Material
{
	float32_t4x4 projectionInverse;
};

Texture2D<float32_t4> gTexture : register(t0);
// Depthを読む
Texture2D<float32_t> gDepthTexture : register(t1);

SamplerState gSamplerLinear : register(s0);
SamplerState gSamplerPoint : register(s1);

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET0;
};

static const float32_t2 kIndex3x3[3][3] =
{
	{ { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
	{ { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

static const float32_t kHorizontalKernel[3][3] =
{
	{ -1.0f / 6.0f, 0, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0, 1.0f / 6.0f }
};

static const float32_t kVerticalKernel[3][3] =
{
	{ -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
	{ 0, 0, 0 },
	{ 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
};

static const float32_t PI = 3.14159265f;

PixelShaderOutput main(VertexShaderOutput input)
{
	uint32_t width, height;
	gTexture.GetDimensions(width, height);
	float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
	
    // 畳み込み
	float32_t2 difference = float32_t2(0.0f, 0.0f);
    
	for (int32_t x = 0; x < 3; ++x)
	{
		for (int32_t y = 0; y < 3; ++y)
		{
			float32_t2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
			float32_t ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
			// depth->ViewZ
			float32_t4 viewSpace = mul(float32_t4(0.0f, 0.0f, ndcDepth, 1.0f), gMaterial.projectionInverse);
			float32_t viewZ = viewSpace.z * rcp(viewSpace.w);

			difference.x += viewZ * kHorizontalKernel[x][y];
			difference.y += viewZ * kVerticalKernel[x][y];
		}
	}
	
	float32_t weight = length(difference);
	weight = saturate(weight);

	PixelShaderOutput output;
	output.color.rgb = (1.0f - weight) * gTexture.Sample(gSamplerLinear, input.texcoord).rgb;
	output.color.a = 1.0f;
    
	return output;
}