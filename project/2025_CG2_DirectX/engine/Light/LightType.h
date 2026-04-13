#pragma once
#include <cstdint>

/// <summary>
/// ライトの種類定義
/// </summary>
enum class LightType {
	Directional,
	Point,
	Spot,
	Area,
};