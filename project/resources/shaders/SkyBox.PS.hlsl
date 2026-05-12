#include "SkyBox.hlsli"

enum Reflectance
{
    None,
	Lambert,
	HalfLambert,
};

enum Reflection
{
    NoneReflection,
	PhongReflection,
	BlinnPhongReflection,
};

enum LightType
{
	Directional,
	Point,
	Spot,
    Area,
};

struct Material{
    float32_t4 color;
    int32_t enableLighting;
    int32_t enableReflection;
    float32_t shininess;
    float32_t4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);

TextureCube<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct LightData
{
    uint32_t type; // Lightの種類
    uint32_t enable; // ライトの有効無効
    float32_t2 size; //サイズ(Area)
    
    float32_t4 color; // ライトの色

    float32_t3 direction; // 向き( Directional ・ Spot )
    float radius; // ライトの届く最大距離( Point ・ Spot)

    float32_t3 position; // 位置(Point ・ Spot)
    float intensity; // 輝度
    
    float decay; // 減衰率( Point ・ Spot )
    float cosAngle; // 余弦( Spot )
    float cosFalloffStart; // 減衰開始角度( Spot )
    float _pad1;
    
};

#define MAX_LIGHTS 128

struct LightBuffer
{
    uint32_t lightCount;
    float32_t3 padding;
    LightData lights[MAX_LIGHTS];
};
ConstantBuffer<LightBuffer> gLightBuffer : register(b1);

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor * gMaterial.color;
    return output;
}