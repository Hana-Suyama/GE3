#pragma once
#include <Matrix4x4.h>

struct PostEffectMaterial
{
    Matrix4x4 projectionInverse;
    float time;
    float padding[3];
};