#pragma once
#include "2025_CG2_DirectX/engine/utility/Math/MyMath.h"
#include "2025_CG2_DirectX/engine/utility/Math/Vector2.h"
#include "2025_CG2_DirectX/engine/utility/Math/Vector3.h"
#include "2025_CG2_DirectX/engine/utility/Math/Vector4.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	int32_t falseUV;
};