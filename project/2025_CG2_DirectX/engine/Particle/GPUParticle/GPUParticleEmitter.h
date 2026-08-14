#pragma once
#include <Vector3.h>
#include <Vector4.h>

enum class ParticleShapeType : uint32_t {
    Point,
    Sphere,
};

struct GPUParticleEmitter {
    Vector3 position;
    float emissionRate;

    Vector3 rotation;
    float emissionAccumulator;

    Vector3 scale;
    uint32_t effectIndex;

    uint32_t shapeType;
    uint32_t randomSeed;
    uint32_t active;
    uint32_t burstCount;

    Vector4 shapeParameter;
};