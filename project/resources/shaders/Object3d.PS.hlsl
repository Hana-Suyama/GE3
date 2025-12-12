#include "object3d.hlsli"

enum Light
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

struct DirectionalLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);

struct PointLight
{
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    float decay;
};
ConstantBuffer<PointLight> gPointLight : register(b3);

struct SpotLight
{
    float32_t4 color;
    float32_t3 position;
    float32_t intensity;
    float32_t3 direction;
    float32_t distance;
    float32_t decay;
    float32_t cosAngle;
    float32_t cosFalloffStart;
};
ConstantBuffer<SpotLight> gSpotLight : register(b4);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(GSOutput input)
{
    PixelShaderOutput output;
    // UVの処理
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // DirectionalLightの拡散反射
    float32_t3 diffuseDirectionalLight;
    if (gMaterial.enableLighting == Lambert)
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        diffuseDirectionalLight = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    }
    else if (gMaterial.enableLighting == HalfLambert)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        diffuseDirectionalLight = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
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
    float32_t3 reflectLightDiffuse = reflect(gDirectionalLight.direction, normalize(input.normal));
    float specularPowDiffuse;
    float32_t3 specularDirectionalLight = float32_t3(0.0f, 0.0f, 0.0f);
    if (gMaterial.enableReflection == PhongReflection)
    {
        float RdotE = dot(reflectLightDiffuse, toEye);
        specularPowDiffuse = pow(saturate(RdotE), gMaterial.shininess);
        specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPowDiffuse * float32_t3(1.0f, 1.0f, 1.0f);
    }
    else if (gMaterial.enableReflection == BlinnPhongReflection)
    {
        float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        specularPowDiffuse = pow(saturate(NDotH), gMaterial.shininess);
        specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPowDiffuse * float32_t3(1.0f, 1.0f, 1.0f);
    }
    
    float32_t distance = length(gPointLight.position - input.worldPosition);
    float32_t factor = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay);
    
    // PointLightの拡散反射
    float32_t3 diffusePointLight = float32_t3(0.0f, 0.0f, 0.0f);
    float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
    if (gMaterial.enableLighting == Lambert)
    {
        float cos = saturate(dot(normalize(input.normal), -pointLightDirection));
        diffusePointLight = gMaterial.color.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
    }
    else if (gMaterial.enableLighting == HalfLambert)
    {
        float NdotL = dot(normalize(input.normal), -pointLightDirection);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        diffusePointLight = gMaterial.color.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
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
    float specularPowPoint;
    float32_t3 specularPointLight = float32_t3(0.0f, 0.0f, 0.0f);
    if (gMaterial.enableReflection == PhongReflection)
    {
        float RdotE = dot(reflectLightPoint, toEye);
        specularPowPoint = pow(saturate(RdotE), gMaterial.shininess);
        specularPointLight = gPointLight.color.rgb * gPointLight.intensity * specularPowPoint * float32_t3(1.0f, 1.0f, 1.0f) * factor;
    }
    else if (gMaterial.enableReflection == BlinnPhongReflection)
    {
        float32_t3 halfVector = normalize(-pointLightDirection + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        specularPowPoint = pow(saturate(NDotH), gMaterial.shininess);
        specularPointLight = gPointLight.color.rgb * gPointLight.intensity * specularPowPoint * float32_t3(1.0f, 1.0f, 1.0f) * factor;
    }
    
    
    float32_t distanceSpot = length(gSpotLight.position - input.worldPosition);
    float32_t factorSpot = pow(saturate(-distanceSpot / gSpotLight.distance + 1.0), gSpotLight.decay);
    
    // SpotLightの拡散反射
    float32_t3 diffuseSpotLight = float32_t3(0.0f, 0.0f, 0.0f);
    float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
    float32_t cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
    float32_t falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
    
    if (gMaterial.enableLighting == Lambert)
    {
        float cos = saturate(dot(normalize(input.normal), -spotLightDirectionOnSurface));
        diffuseSpotLight = gMaterial.color.rgb * gSpotLight.color.rgb * cos * gSpotLight.intensity * factorSpot * falloffFactor;
    }
    else if (gMaterial.enableLighting == HalfLambert)
    {
        float NdotL = dot(normalize(input.normal), -spotLightDirectionOnSurface);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        diffuseSpotLight = gMaterial.color.rgb * gSpotLight.color.rgb * cos * gSpotLight.intensity * factorSpot * falloffFactor;
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
        specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * specularPowSpot * float32_t3(1.0f, 1.0f, 1.0f) * factorSpot * falloffFactor;
    }
    else if (gMaterial.enableReflection == BlinnPhongReflection)
    {
        float32_t3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        specularPowSpot = pow(saturate(NDotH), gMaterial.shininess);
        specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * specularPowSpot * float32_t3(1.0f, 1.0f, 1.0f) * factorSpot * falloffFactor;
    }
    
    // 透明度の計算
    output.color.a = gMaterial.color.a;
    if (!input.falseUV)
    {
        output.color.a *= textureColor.a;
    }
    
    output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
    
    if (textureColor.a == 0.0f){
        discard;
    }
    
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}