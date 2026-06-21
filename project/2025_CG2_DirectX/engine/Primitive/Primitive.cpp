#include "Primitive.h"
#include <numbers>

ParticleMeshData Primitive::CreatePoint()
{
	ParticleMeshData meshData;
	meshData.vertices.resize(1);

	meshData.vertices[0].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	meshData.vertices[0].texcoord = { 0.0f, 1.0f };
	meshData.vertices[0].normal = { 0.0f, 0.0f, -1.0f };

	return meshData;
}

ParticleMeshData Primitive::CreateQuad(float width, float height)
{
	ParticleMeshData meshData;
	meshData.vertices.resize(4);

	// HitEffect用
	meshData.vertices[0].position = { -width * 0.5f, -height * 0.5f, 0.0f, 1.0f };
	meshData.vertices[0].texcoord = { 0.0f, 1.0f };

	meshData.vertices[1].position = { -width * 0.5f,  height * 0.5f, 0.0f, 1.0f };
	meshData.vertices[1].texcoord = { 0.0f, 0.0f };

	meshData.vertices[2].position = {  width * 0.5f, -height * 0.5f, 0.0f, 1.0f };
	meshData.vertices[2].texcoord = { 1.0f, 1.0f };

	meshData.vertices[3].position = {  width * 0.5f,  height * 0.5f, 0.0f, 1.0f };
	meshData.vertices[3].texcoord = { 1.0f, 0.0f };

	for (uint32_t i = 0; i < 4; ++i) {
		meshData.vertices[i].normal = { 0.0f, 0.0f, -1.0f };
		meshData.vertices[i].falseUV = false;
	}
	return meshData;
}

ParticleMeshData Primitive::CreateRing(uint32_t divide, float outerRadius, float innerRadius)
{
	ParticleMeshData meshData;
	meshData.vertices.resize(divide * 6);

	float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

	for (uint32_t index = 0; index < divide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(divide);
		float uNext = float(index + 1) / float(divide);

		meshData.vertices[index * 6].position = { -sin * outerRadius, cos * outerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6].texcoord = { u, 0.0f };
		meshData.vertices[index * 6 + 1].position = { -sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6 + 1].texcoord = { uNext, 0.0f };
		meshData.vertices[index * 6 + 2].position = { -sin * innerRadius, cos * innerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6 + 2].texcoord = { u, 1.0f };

		meshData.vertices[index * 6 + 3].position = { -sin * innerRadius, cos * innerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6 + 3].texcoord = { u, 1.0f };
		meshData.vertices[index * 6 + 4].position = { -sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6 + 4].texcoord = { uNext, 0.0f };
		meshData.vertices[index * 6 + 5].position = { -sinNext * innerRadius, cosNext * innerRadius, 0.0f, 1.0f };
		meshData.vertices[index * 6 + 5].texcoord = { uNext, 1.0f };
	}

	for (uint32_t i = 0; i < divide * 6; ++i) {
		meshData.vertices[i].normal = { 0.0f, 0.0f, 1.0f };
		meshData.vertices[i].falseUV = false;
	}
    return meshData;
}

ParticleMeshData Primitive::CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height)
{
	ParticleMeshData meshData;
	meshData.vertices.resize(divide * 6);

	float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

	for (uint32_t index = 0; index < divide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(divide);
		float uNext = float(index + 1) / float(divide);

		meshData.vertices[index * 6].position = { -sin * topRadius, height, cos * topRadius, 1.0f };
		meshData.vertices[index * 6].texcoord = { u, 0.0f };
		meshData.vertices[index * 6].normal = { -sin, 0.0f, cos };
		meshData.vertices[index * 6 + 1].position = { -sinNext * topRadius, height, cosNext * topRadius, 1.0f };
		meshData.vertices[index * 6 + 1].texcoord = { uNext, 0.0f };
		meshData.vertices[index * 6 + 1].normal = { -sinNext, 0.0f, cosNext };
		meshData.vertices[index * 6 + 2].position = { -sin * bottomRadius, 0.0f, cos * bottomRadius, 1.0f };
		meshData.vertices[index * 6 + 2].texcoord = { u, 1.0f };
		meshData.vertices[index * 6 + 2].normal = { -sin, 0.0f, cos };
		meshData.vertices[index * 6 + 3].position = { -sin * bottomRadius, 0.0f, cos * bottomRadius, 1.0f };
		meshData.vertices[index * 6 + 3].texcoord = { u, 1.0f };
		meshData.vertices[index * 6 + 3].normal = { -sin, 0.0f, cos };
		meshData.vertices[index * 6 + 4].position = { -sinNext * topRadius, height, cosNext * topRadius, 1.0f };
		meshData.vertices[index * 6 + 4].texcoord = { uNext, 0.0f };
		meshData.vertices[index * 6 + 4].normal = { -sinNext, 0.0f, cosNext };
		meshData.vertices[index * 6 + 5].position = { -sinNext * bottomRadius, 0.0f, cosNext * bottomRadius, 1.0f };
		meshData.vertices[index * 6 + 5].texcoord = { uNext, 1.0f };
		meshData.vertices[index * 6 + 5].normal = { -sinNext, 0.0f, cosNext };
	}
    return meshData;
}
