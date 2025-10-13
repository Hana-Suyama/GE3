#pragma once

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