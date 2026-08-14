#include "ParticleCS.hlsli"

struct Emitter
{
    float32_t3 position;
    float32_t emissionRate;

    float32_t3 rotation;
    float32_t emissionAccumulator;

    float32_t3 scale;
    uint32_t effectIndex;

    uint32_t shapeType;
    uint32_t randomSeed;
    uint32_t active;
    uint32_t burstCount;

    float32_t4 shapeParameter;
};

float32_t rand3dTo1d(
    float32_t3 value,
    float32_t3 dotDir = float32_t3(12.9898f, 78.233f, 37.719f))
{
    float32_t3 smallValue = sin(value);
    float32_t random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453f);
    return random;
}

float32_t3 rand3dTo3d(float32_t3 value)
{
    return float32_t3(
        rand3dTo1d(value, float32_t3(12.9898f, 78.233f, 37.719f)),
        rand3dTo1d(value, float32_t3(39.3468f, 11.135f, 83.155f)),
        rand3dTo1d(value, float32_t3(73.156f, 52.235f, 9.151f)));
}

class RandomGenerator
{
    float32_t3 seed;
    float32_t3 Generate3d() {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float32_t Generate1d() {
        float32_t result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
};

RWStructuredBuffer<Particle> gParticles : register(u0);
ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

[numthreads(1, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID)
{
    if (gEmitter.emit != 0)
    {
        RandomGenerator generator;
        generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
        for (uint32_t countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            int32_t freeListIndex;
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < kMaxParticles)
            {
                uint32_t particleIndex = gFreeList[freeListIndex];
                gParticles[particleIndex].scale = generator.Generate3d();
                gParticles[particleIndex].translate = generator.Generate3d();
                gParticles[particleIndex].color.rgb = generator.Generate3d();
                gParticles[particleIndex].color.a = 1.0f;
                gParticles[particleIndex].lifeTime = 2.0f;
                gParticles[particleIndex].velocity = (generator.Generate3d() * 2.0f - 1.0f) * 0.5f;
                gParticles[particleIndex].currentTime = 0.0f;
            }
            else
            {
                InterlockedAdd(gFreeListIndex[0], 1);
                break;
            }
        }

    }
}
