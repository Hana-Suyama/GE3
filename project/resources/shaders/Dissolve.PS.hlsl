#include "Dissolve.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
Texture2D<float32_t> gMaskTexture : register(t2);

struct PostEffectMaterial
{
    float32_t4x4 projectionInverse;
    float32_t time;
    float32_t threshold;
    float32_t2 padding;
};

ConstantBuffer<PostEffectMaterial> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    float32_t mask = gMaskTexture.Sample(gSampler, input.texcoord);
    
    if (mask <= gMaterial.threshold)
    {
        discard;
    }
    
    float32_t edge = 1.0f - smoothstep(0.5f, 0.53f, mask);
    output.color = gTexture.Sample(gSampler, input.texcoord);
    output.color.rgb += edge * float32_t3(1.0f, 0.4f, 0.3f);
    
    return output;
}