#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <cstdint>

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	int32_t falseUV;
};
