#pragma once
#include "cmath"

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 {

	/* --------- 本体 --------- */

	float x;
	float y;
	float z;
	float w;

	/* --------- 演算子オーバーロード --------- */

	Vector4 operator+(const Vector4& v1) const;
	Vector4 operator-(const Vector4& v1) const;
	Vector4 operator*(const float s) const;
	Vector4 operator/(const float s) const;

	Vector4 operator-() const;
	Vector4 operator+() const;

	Vector4& operator*=(const float s);
	Vector4& operator-=(const Vector4& v);
	Vector4& operator+=(const Vector4& v);
	Vector4& operator/=(const float s);

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// Vector4の加法
	/// </summary>
	Vector4 Add(const Vector4& a) const;

	/// <summary>
	/// Vector4の減法
	/// </summary>
	Vector4 Subtract(const Vector4& a) const;

	/// <summary>
	/// Vector4の積
	/// </summary>
	Vector4 Multiply(const Vector4& a) const;

	/// <summary>
	/// Vector4のスカラー倍
	/// </summary>
	Vector4 Multiply(const float s) const;

	/* --------- 数学 --------- */

	/// <summary>
	/// ノルム
	/// </summary>
	float Norm() const;

	/// <summary>
	/// 正規化
	/// </summary>
	Vector4 Normalize() const;

	/// <summary>
	/// 内積
	/// </summary>
	float Dot(const Vector4 v1) const;

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Vector4 Vector4::operator+(const Vector4& v1) const { return Add(v1); }
inline Vector4 Vector4::operator-(const Vector4& v1) const { return Subtract(v1); }
inline Vector4 Vector4::operator*(const float s) const { return Multiply(s); }
inline Vector4 operator*(float s, const Vector4& v) { return v * s; }
inline Vector4 Vector4::operator/(const float s) const { return Multiply(1.0f / s); }

inline Vector4 Vector4::operator-() const { return { -x, -y, -z, -w }; }
inline Vector4 Vector4::operator+() const { return { x, y, z, w }; }

inline Vector4& Vector4::operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
inline Vector4& Vector4::operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
inline Vector4& Vector4::operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
inline Vector4& Vector4::operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }


/// <summary>
/// Vector4の加法
/// </summary>
inline Vector4 Vector4::Add(const Vector4& a) const {
	Vector4 result{};
	result.x = x + a.x;
	result.y = y + a.y;
	result.z = z + a.z;
	result.w = w + a.w;
	return result;
}

/// <summary>
/// Vector4の減法
/// </summary>
inline Vector4 Vector4::Subtract(const Vector4& a) const {
	Vector4 result{};
	result.x = x - a.x;
	result.y = y - a.y;
	result.z = z - a.z;
	result.w = w - a.w;
	return result;
}

/// <summary>
/// Vector4の積
/// </summary>
inline Vector4 Vector4::Multiply(const Vector4& a) const {
	Vector4 result{};
	result.x = x * a.x;
	result.y = y * a.y;
	result.z = z * a.z;
	result.w = w * a.w;
	return result;
}

/// <summary>
/// Vector4のスカラー倍
/// </summary>
inline Vector4 Vector4::Multiply(const float s) const {
	Vector4 result{};
	result.x = x * s;
	result.y = y * s;
	result.z = z * s;
	result.w = w * s;
	return result;
}

/// <summary>
/// ノルム
/// </summary>
inline float Vector4::Norm() const {
	float result = sqrtf(x * x + y * y + z * z + w * w);
	return result;
}

/// <summary>
/// 正規化
/// </summary>
inline Vector4 Vector4::Normalize() const {

	Vector4 result{};

	float length = Norm();

	if (length != 0.0f) {
		result.x = x / length;
		result.y = y / length;
		result.z = z / length;
		result.w = w / length;
	}

	return result;
}

/// <summary>
/// 内積
/// </summary>
inline float Vector4::Dot(const Vector4 v1) const {
	float result{};
	result = (x * v1.x) + (y * v1.y) + (z * v1.z) + (w * v1.w);
	return result;
}
