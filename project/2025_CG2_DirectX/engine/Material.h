#pragma once
#include "utility/Math/Vector4.h"
#include "utility/Math/Matrix4x4.h"

struct Material {
	Vector4 color;
	int32_t enableLighting;
	int32_t enableReflection;
	float shininess;
	float padding[1];
	Matrix4x4 uvTransform;
};

enum Light {
	None,
	Lambert,
	HalfLambert,
};

enum Reflection {
	NoneReflection,
	PhongReflection,
	BlinnPhongReflection,
};
