#pragma once
#include "cmath"

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 {

	/* --------- 本体 --------- */

	float x;
	float y;

	/* --------- 演算子オーバーロード --------- */

	Vector2 operator+(const Vector2& v1) const;
	Vector2 operator-(const Vector2& v1) const;
	Vector2 operator*(const float s) const;
	Vector2 operator/(const float s) const;

	Vector2 operator-() const;
	Vector2 operator+() const;

	Vector2& operator*=(const float s);
	Vector2& operator-=(const Vector2& v);
	Vector2& operator+=(const Vector2& v);
	Vector2& operator/=(const float s);

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// Vector2の加法
	/// </summary>
	Vector2 Add(const Vector2& a) const;

	/// <summary>
	/// Vector2の減法
	/// </summary>
	Vector2 Subtract(const Vector2& a) const;

	/// <summary>
	/// Vector2の積
	/// </summary>
	Vector2 Multiply(const Vector2& a) const;

	/// <summary>
	/// Vector2のスカラー倍
	/// </summary>
	Vector2 Multiply(const float s) const;

	/* --------- 数学 --------- */

	/// <summary>
	/// 正規化
	/// </summary>
	Vector2 Normalize() const;

	/// <summary>
	/// 内積
	/// </summary>
	float Dot(const Vector2 v1) const;

	/// <summary>
	/// クロス積
	/// </summary>
	float Cross(const Vector2& v1) const;

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Vector2 Vector2::operator+(const Vector2& v1) const { return Add(v1); }
inline Vector2 Vector2::operator-(const Vector2& v1) const { return Subtract(v1); }
inline Vector2 Vector2::operator*(const float s) const { return Multiply(s); }
inline Vector2 operator*(float s, const Vector2& v) { return v * s; }
inline Vector2 Vector2::operator/(const float s) const { return Multiply(1.0f / s); }

inline Vector2 Vector2::operator-() const { return { -x, -y }; }
inline Vector2 Vector2::operator+() const { return { x, y }; }

inline Vector2& Vector2::operator*=(float s) { x *= s; y *= s; return *this; }
inline Vector2& Vector2::operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
inline Vector2& Vector2::operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
inline Vector2& Vector2::operator/=(float s) { x /= s; y /= s; return *this; }


/// <summary>
/// Vector2の加法
/// </summary>
inline Vector2 Vector2::Add(const Vector2& a) const {
	Vector2 result{};
	result.x = x + a.x;
	result.y = y + a.y;
	return result;
}

/// <summary>
/// Vector2の減法
/// </summary>
inline Vector2 Vector2::Subtract(const Vector2& a) const {
	Vector2 result{};
	result.x = x - a.x;
	result.y = y - a.y;
	return result;
}

/// <summary>
/// Vector2の積
/// </summary>
inline Vector2 Vector2::Multiply(const Vector2& a) const {
	Vector2 result{};
	result.x = x * a.x;
	result.y = y * a.y;
	return result;
}

/// <summary>
/// Vector2のスカラー倍
/// </summary>
inline Vector2 Vector2::Multiply(const float s) const {
	Vector2 result{};
	result.x = x * s;
	result.y = y * s;
	return result;
}

/// <summary>
/// 正規化
/// </summary>
inline Vector2 Vector2::Normalize() const {

	Vector2 result{};

	float length = sqrtf(x * x + y * y);

	if (length != 0.0f) {
		result.x = x / length;
		result.y = y / length;
	}

	return result;
}

/// <summary>
/// 内積
/// </summary>
inline float Vector2::Dot(const Vector2 v1) const {
	float Result{};
	Result = (x * v1.x) + (y * v1.y);
	return Result;
}

/// <summary>
/// クロス積
/// </summary>
inline float Vector2::Cross(const Vector2& v1) const {
	float result{};
	result = (x * v1.y) - (y * v1.x);
	return result;
}

