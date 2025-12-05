#pragma once
#include "utility/Math/MyMath.h"
#include "utility/Math/Vector2.h"
#include "utility/Math/Vector3.h"
#include "utility/Math/Vector4.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	int32_t falseUV;
};