#pragma once
#include <vector>
#include <DirectXBasic.h>
#include <VertexData.h>

struct ParticleMeshData {
    std::vector<VertexData> vertices;
    D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

class Primitive
{
public:
    static ParticleMeshData CreatePoint();
	static ParticleMeshData CreateQuad(float width, float height);
    static ParticleMeshData CreateRing(uint32_t divide, float outerRadius, float innerRadius);
    static ParticleMeshData CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height);
};

