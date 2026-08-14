#pragma once
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

struct GpuParticleEffect {
    Vector3 velocityMin;
    float lifetimeMin;

    Vector3 velocityMax;
    float lifetimeMax;

    Vector2 sizeMin;
    Vector2 sizeMax;

    Vector4 startColor;
    Vector4 endColor;

    Vector3 acceleration;
    float drag;
};