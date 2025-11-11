#pragma once
#include "2025_CG2_DirectX/engine/utility/Math/MyMath.h"
#include "2025_CG2_DirectX/engine/utility/Math/Matrix4x4.h"

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};