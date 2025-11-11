#pragma once
#include "2025_CG2_DirectX/engine/utility/Math/Vector4.h"
#include "2025_CG2_DirectX/engine/utility/Math/Matrix4x4.h"

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

enum Light {
	None,
	Lambert,
	HalfLambert,
};