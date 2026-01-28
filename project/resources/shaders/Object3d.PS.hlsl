#include "object3d.hlsli"

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
};

struct Material{
    float32_t4 color;
    int32_t enableLighting;
    int32_t enableReflection;
    float32_t shininess;
    float32_t4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct LightData
{
    uint32_t type; // Lightの種類
    uint32_t enable; // ライトの有効無効
    float32_t2 _pad0;
    
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

PixelShaderOutput main(GSOutput input)
{
    PixelShaderOutput output;
    output.color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // UVの処理
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // ライトの数だけfor文を回す
    for (int32_t i = 0; i < gLightBuffer.lightCount; i++)
    {
        // ライトが無効ならスキップ
        if (gLightBuffer.lights[i].enable == 0)
        {
            continue;
        }
        
        if (gLightBuffer.lights[i].type == Directional)
        {
            // DirectionalLightの拡散反射
            float32_t3 diffuseDirectionalLight;
            if (gMaterial.enableLighting == Lambert)
            {
                float cos = saturate(dot(normalize(input.normal), -gLightBuffer.lights[i].direction));
                diffuseDirectionalLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity;
            }
            else if (gMaterial.enableLighting == HalfLambert)
            {
                float NdotL = dot(normalize(input.normal), -gLightBuffer.lights[i].direction);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
                diffuseDirectionalLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity;
            }
            else
            {
                diffuseDirectionalLight = gMaterial.color.rgb;
            }
           // UVがあったらtexturecolorをかける
            if (!input.falseUV)
            {
                diffuseDirectionalLight *= textureColor.rgb;
            }
    
            // DirectionalLightの鏡面反射
            // 鏡面反射の計算
            // カメラからの向き
            float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float32_t3 reflectLightDiffuse = reflect(gLightBuffer.lights[i].direction, normalize(input.normal));
            float specularPowDiffuse;
            float32_t3 specularDirectionalLight = float32_t3(0.0f, 0.0f, 0.0f);
            if (gMaterial.enableReflection == PhongReflection)
            {
                float RdotE = dot(reflectLightDiffuse, toEye);
                specularPowDiffuse = pow(saturate(RdotE), gMaterial.shininess);
                specularDirectionalLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowDiffuse * float32_t3(1.0f, 1.0f, 1.0f);
            }
            else if (gMaterial.enableReflection == BlinnPhongReflection)
            {
                float32_t3 halfVector = normalize(-gLightBuffer.lights[i].direction + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                specularPowDiffuse = pow(saturate(NDotH), gMaterial.shininess);
                specularDirectionalLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowDiffuse * float32_t3(1.0f, 1.0f, 1.0f);
            }
            
            output.color.rgb += diffuseDirectionalLight + specularDirectionalLight;
    
        }
        else if (gLightBuffer.lights[i].type == Point)
        {
            
            float32_t distance = length(gLightBuffer.lights[i].position - input.worldPosition);
            float32_t factor = pow(saturate(-distance / gLightBuffer.lights[i].radius + 1.0), gLightBuffer.lights[i].decay);
            
            // PointLightの拡散反射
            float32_t3 diffusePointLight = float32_t3(0.0f, 0.0f, 0.0f);
            float32_t3 pointLightDirection = normalize(input.worldPosition - gLightBuffer.lights[i].position);
            if (gMaterial.enableLighting == Lambert)
            {
                float cos = saturate(dot(normalize(input.normal), -pointLightDirection));
                diffusePointLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity * factor;
            }
            else if (gMaterial.enableLighting == HalfLambert)
            {
                float NdotL = dot(normalize(input.normal), -pointLightDirection);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
                diffusePointLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity * factor;
            }
           // UVがあったらtexturecolorをかける
            if (!input.falseUV)
            {
               diffusePointLight *= textureColor.rgb;
            }
    
            // PointLightの鏡面反射
            // 鏡面反射の計算
            // カメラからの向き
            float32_t3 reflectLightPoint = reflect(pointLightDirection, normalize(input.normal));
            float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float specularPowPoint;
            float32_t3 specularPointLight = float32_t3(0.0f, 0.0f, 0.0f);
            if (gMaterial.enableReflection == PhongReflection)
            {
                float RdotE = dot(reflectLightPoint, toEye);
                specularPowPoint = pow(saturate(RdotE), gMaterial.shininess);
                specularPointLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowPoint * float32_t3(1.0f, 1.0f, 1.0f) * factor;
            }
            else if (gMaterial.enableReflection == BlinnPhongReflection)
            {
                float32_t3 halfVector = normalize(-pointLightDirection + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                specularPowPoint = pow(saturate(NDotH), gMaterial.shininess);
                specularPointLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowPoint * float32_t3(1.0f, 1.0f, 1.0f) * factor;
            }
            
            output.color.rgb += diffusePointLight + specularPointLight;

        }
        else if (gLightBuffer.lights[i].type == Spot)
        {
            
            float32_t distanceSpot = length(gLightBuffer.lights[i].position - input.worldPosition);
            float32_t factorSpot = pow(saturate(-distanceSpot / gLightBuffer.lights[i].radius + 1.0), gLightBuffer.lights[i].decay);
    
            // SpotLightの拡散反射
            float32_t3 diffuseSpotLight = float32_t3(0.0f, 0.0f, 0.0f);
            float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - gLightBuffer.lights[i].position);
            float32_t cosAngle = dot(spotLightDirectionOnSurface, gLightBuffer.lights[i].direction);
            float32_t falloffFactor = saturate((cosAngle - gLightBuffer.lights[i].cosAngle) / (gLightBuffer.lights[i].cosFalloffStart - gLightBuffer.lights[i].cosAngle));
    
            if (gMaterial.enableLighting == Lambert)
            {
                float cos = saturate(dot(normalize(input.normal), -spotLightDirectionOnSurface));
                diffuseSpotLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity * factorSpot * falloffFactor;
            }
            else if (gMaterial.enableLighting == HalfLambert)
            {
                float NdotL = dot(normalize(input.normal), -spotLightDirectionOnSurface);
                float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
                diffuseSpotLight = gMaterial.color.rgb * gLightBuffer.lights[i].color.rgb * cos * gLightBuffer.lights[i].intensity * factorSpot * falloffFactor;
            }
           // UVがあったらtexturecolorをかける
            if (!input.falseUV)
            {
                diffuseSpotLight *= textureColor.rgb;
            }
    
            // SpotLightの鏡面反射
            // 鏡面反射の計算
            // カメラからの向き
            float32_t3 reflectLightSpot = reflect(spotLightDirectionOnSurface, normalize(input.normal));
            float specularPowSpot;
            float32_t3 specularSpotLight = float32_t3(0.0f, 0.0f, 0.0f);
            if (gMaterial.enableReflection == PhongReflection)
            {
                float RdotE = dot(reflectLightSpot, toEye);
                specularPowSpot = pow(saturate(RdotE), gMaterial.shininess);
                specularSpotLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowSpot * float32_t3(1.0f, 1.0f, 1.0f) * factorSpot * falloffFactor;
            }
            else if (gMaterial.enableReflection == BlinnPhongReflection)
            {
                float32_t3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
                float NDotH = dot(normalize(input.normal), halfVector);
                specularPowSpot = pow(saturate(NDotH), gMaterial.shininess);
                specularSpotLight = gLightBuffer.lights[i].color.rgb * gLightBuffer.lights[i].intensity * specularPowSpot * float32_t3(1.0f, 1.0f, 1.0f) * factorSpot * falloffFactor;
            }
            
            output.color.rgb += diffuseSpotLight + specularSpotLight;
        }
        
    }
    
    // 透明度の計算
    output.color.a = gMaterial.color.a;
    if (!input.falseUV)
    {
        output.color.a *= textureColor.a;
    }
    
    if (textureColor.a == 0.0f){
        discard;
    }
    
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}