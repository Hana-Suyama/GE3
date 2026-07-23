struct Particle
{
    float32_t3 translate;
    float32_t3 scale;
    float32_t lifeTime;
    float32_t3 velocity;
    float32_t currentTime;
    float32_t4 color;
};

struct PerView
{
    float32_t4x4 viewProjection;
    float32_t4x4 billboardMatrix;
};
