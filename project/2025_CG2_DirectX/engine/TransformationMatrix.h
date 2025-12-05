#pragma once
#include "utility/Math/MyMath.h"
#include "utility/Math/Matrix4x4.h"

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};