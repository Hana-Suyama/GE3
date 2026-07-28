#pragma once
#include <Matrix4x4.h>

struct PostEffectMaterial
{
    Matrix4x4 projectionInverse;
    float time;
    float threshold;
    float padding[2];
};