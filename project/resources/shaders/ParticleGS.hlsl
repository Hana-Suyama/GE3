#include "Particle.hlsli"

[maxvertexcount(4)]
void main(
	point VertexShaderOutput input[1],
	inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
    // 共通
    element.normal = input[0].normal;
    element.falseUV = input[0].falseUV;
    element.color = input[0].color;
    
    // 1点目
    element.position = input[0].position + float32_t4(-2.0f, -2.0f, 0, 0);
    element.texcoord = float32_t2(0.0f, 0.0f);
    output.Append(element);
    
    // 2点目
    element.position = input[0].position + float32_t4(-2.0f, 2.0f, 0, 0);
    element.texcoord = float32_t2(0.0f, 1.0f);
    output.Append(element);
    
    // 2点目
    element.position = input[0].position + float32_t4(2.0f, -2.0f, 0, 0);
    element.texcoord = float32_t2(1.0f, 0.0f);
    output.Append(element);
    
    element.position = input[0].position + float32_t4(2.0f, 2.0f, 0, 0);
    element.texcoord = float32_t2(1.0f, 1.0f);
    output.Append(element);
}