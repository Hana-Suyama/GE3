#pragma once
#include <cstdint>
#include <Vector3.h>
#include <Vector4.h>

const uint32_t kMaxLights = 128; // 仮最大ライト数。あとでエンジン側でまとめる

/// <summary>
/// GPUに一括で送る用のライト1つ分の構造体(全部込み)
/// </summary>
struct LightData {
	uint32_t type;      // Lightの種類
	uint32_t enable;	// ライトの有効無効
	float _pat0[2];		// パディング

	Vector4 color;		// ライトの色

	Vector3 direction;	// 向き( Directional ・ Spot )
	float radius;       // ライトの届く最大距離( Point ・ Spot)

	Vector3 position;  // 位置(Point ・ Spot)
	float intensity;	// 輝度

	float decay;		// 減衰率( Point ・ Spot )
	float cosAngle;		// 余弦( Spot )
	float cosFalloffStart;	// 減衰開始角度( Spot )
	float _pad1;		// パディング

};

/// <summary>
/// GPUに一括でライト全部送る用のライトデータ構造体
/// </summary>
struct LightBuffer {
	uint32_t lightCount;
	float padding[3];
	LightData lights[kMaxLights];
};