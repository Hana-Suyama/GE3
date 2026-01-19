#pragma once

/// <summary>
/// 線形補間(テンプレート)
/// </summary>
namespace MyMath {

	template<class T>
	T Lerp(const T& a, const T& b, float t) {
		return a + (b - a) * t;
	}

}