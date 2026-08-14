struct Particle
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

struct PerView
{
    float32_t4x4 viewProjection;
    float32_t4x4 billboardMatrix;
};

struct PerFrame
{
    float32_t time;
    float32_t deltaTime;
};


static const uint32_t kMaxParticles = 1024;
