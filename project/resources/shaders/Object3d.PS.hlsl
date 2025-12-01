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

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
    // 鏡面反射の計算
    float specularPow;
    if (gMaterial.enableReflection == PhongReflection)
    {
        float RdotE = dot(reflectLight, toEye);
        specularPow = pow(saturate(RdotE), gMaterial.shininess);
    }
    else if (gMaterial.enableReflection == BlinnPhongReflection)
    {
        float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        specularPow = pow(saturate(NDotH), gMaterial.shininess);
    }
    
    if (textureColor.a == 0.0f){
        discard;
    }
    
    float cos;
    if (gMaterial.enableLighting == Lambert)
    {
        cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
    }
    else if (gMaterial.enableLighting == HalfLambert)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    
    // 鏡面反射
    float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    
    if (gMaterial.enableLighting == Lambert)
    {
        if (input.falseUV) {
            // 拡散反射
            float32_t3 diffuse = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a;
            
        } else {
            // 拡散反射
            float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
    } else if (gMaterial.enableLighting == HalfLambert){//Lightingする場合
        if (input.falseUV){
            // 拡散反射
            float32_t3 diffuse = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a;
        } else{
            // 拡散反射
            float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.rgb = diffuse + specular;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
    }else{//Lightingしない場合
        if (input.falseUV){
            output.color = gMaterial.color;
        } else{
            output.color = gMaterial.color * textureColor;
        }
    }
    
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}