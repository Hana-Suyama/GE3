#include "Fullscreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct PostEffectMaterial
{
    float32_t4x4 projectionInverse;
    float32_t time;
    float32_t threshold;
    float32_t2 padding;
};

ConstantBuffer<PostEffectMaterial> gMaterial : register(b0);

float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233))
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t random = rand2dTo1d(input.texcoord * gMaterial.time);
    float32_t3 baseColor = gTexture.Sample(gSampler, input.texcoord).rgb;
    float32_t noise = random * 2.0f - 1.0f;
    float32_t noiseStrength = 0.8f;

    output.color.rgb = baseColor * (1.0f + noise * noiseStrength);
    output.color.rgb = saturate(output.color.rgb);
    output.color.a = 1.0f;
    return output;
}