#include "object3d.hlsli"

[maxvertexcount(3)]
void main(
	triangle VertexShaderOutput input[3],
	inout TriangleStream<GSOutput> output
)
{
    for (uint i = 0; i < 3; i++)
    {
        GSOutput element;
        element.position = input[i].position;
        element.normal = input[i].normal;
        element.texcoord = input[i].texcoord;
        element.falseUV = input[i].falseUV;
        element.worldPosition = input[i].worldPosition;
        output.Append(element);
    }
}

// 三角形の入力から点を1つ出力するサンプル
//[maxvertexcount(1)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout PointStream<GSOutput> output
//)
//{
//    GSOutput element;
//    element.position = input[0].position;
//    element.normal = input[0].normal;
//    element.texcoord = input[0].texcoord;
//    element.falseUV = input[0].falseUV;
//    element.worldPosition = input[0].worldPosition;
//    output.Append(element);
//}

// 三角形の入力から点を3つ出力するサンプル
//[maxvertexcount(3)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout PointStream<GSOutput> output
//)
//{
//    for (uint i = 0; i < 3; i++)
//    {
//        GSOutput element;
//        element.position = input[i].position;
//        element.normal = input[i].normal;
//        element.texcoord = input[i].texcoord;
//        element.falseUV = input[i].falseUV;
//        element.worldPosition = input[i].worldPosition;
//        output.Append(element);
//    }
//}

// 三角形の入力から線分を1つ出力するサンプル
//[maxvertexcount(2)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout LineStream<GSOutput> output
//)
//{
//    GSOutput element;
//    element.position = input[0].position;
//    element.normal = input[0].normal;
//    element.texcoord = input[0].texcoord;
//    element.falseUV = input[0].falseUV;
//    element.worldPosition = input[0].worldPosition;
//    output.Append(element);
    
//    element.position = input[1].position;
//    element.normal = input[1].normal;
//    element.texcoord = input[1].texcoord;
//    element.falseUV = input[1].falseUV;
//    element.worldPosition = input[1].worldPosition;
//    output.Append(element);
//}

// 三角形の入力から線分を3つ出力するサンプル
//[maxvertexcount(4)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout LineStream<GSOutput> output
//)
//{
//    GSOutput element;
    
//    for (uint i = 0; i < 3; i++)
//    {
//        element.position = input[i].position;
//        element.normal = input[i].normal;
//        element.texcoord = input[i].texcoord;
//        element.falseUV = input[i].falseUV;
//        element.worldPosition = input[i].worldPosition;
//        output.Append(element);
//    }
    
//    element.position = input[0].position;
//    element.normal = input[0].normal;
//    element.texcoord = input[0].texcoord;
//    element.falseUV = input[0].falseUV;
//    element.worldPosition = input[0].worldPosition;
//    output.Append(element);
//}

// 途中でUVを加工してタイリングするサンプル
//[maxvertexcount(3)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout TriangleStream<GSOutput> output
//)
//{
//    for (uint i = 0; i < 3; i++)
//    {
//        GSOutput element;
//        element.position = input[i].position;
//        element.normal = input[i].normal;
//        element.texcoord = input[i].texcoord * 2.0f;
//        element.falseUV = input[i].falseUV;
//        element.worldPosition = input[i].worldPosition;
//        output.Append(element);
//    }
//}

// 三角形の入力から、三角形を2つ出力
//[maxvertexcount(6)]
//void main(
//	triangle VertexShaderOutput input[3],
//	inout TriangleStream<GSOutput> output
//)
//{
//    // 1つ目の三角形
//    for (uint i = 0; i < 3; i++)
//    {
//        GSOutput element;
//        element.position = input[i].position;
//        element.normal = input[i].normal;
//        element.texcoord = input[i].texcoord;
//        element.falseUV = input[i].falseUV;
//        element.worldPosition = input[i].worldPosition;
//        output.Append(element);
//    }
//    // 現在のストリップを終了
//    output.RestartStrip();
    
//    // 2つ目の三角形
//    for (uint i = 0; i < 3; i++)
//    {
//        GSOutput element;
//        element.position = input[i].position + float32_t4(10.0f, 0.0f, 0.0f, 0.0f);
//        element.normal = input[i].normal;
//        element.texcoord = input[i].texcoord * 5.0f;
//        element.falseUV = input[i].falseUV;
//        element.worldPosition = input[i].worldPosition + float32_t3(10.0f, 0.0f, 0.0f);
//        output.Append(element);
//    }
    
//}