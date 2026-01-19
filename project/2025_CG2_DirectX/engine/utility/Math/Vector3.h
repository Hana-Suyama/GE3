#pragma once
#include "cmath"

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 {

	/* --------- 本体 --------- */

	float x;
	float y;
	float z;

	/* --------- 演算子オーバーロード --------- */

	Vector3 operator+(const Vector3& v1) const;
	Vector3 operator-(const Vector3& v1) const;
	Vector3 operator*(const float s) const;
	Vector3 operator/(const float s) const;

	Vector3 operator-() const;
	Vector3 operator+() const;

	Vector3& operator*=(const float s);
	Vector3& operator-=(const Vector3& v);
	Vector3& operator+=(const Vector3& v);
	Vector3& operator/=(const float s);

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// Vector3の加法
	/// </summary>
	Vector3 Add(const Vector3& a) const;

	/// <summary>
	/// Vector3の減法
	/// </summary>
	Vector3 Subtract(const Vector3& a) const;

	/// <summary>
	/// Vector3の積
	/// </summary>
	Vector3 Multiply(const Vector3& a) const;

	/// <summary>
	/// Vector3のスカラー倍
	/// </summary>
	Vector3 Multiply(const float s) const;

	/* --------- 数学 --------- */

	/// <summary>
	/// ノルム
	/// </summary>
	float Norm() const;

	/// <summary>
	/// 正規化
	/// </summary>
	Vector3 Normalize() const;

	/// <summary>
	/// 内積
	/// </summary>
	float Dot(const Vector3 v1) const;

	/// <summary>
	/// クロス積
	/// </summary>
	Vector3 Cross(const Vector3& v1) const;

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Vector3 Vector3::operator+(const Vector3& v1) const { return Add(v1); }
inline Vector3 Vector3::operator-(const Vector3& v1) const { return Subtract(v1); }
inline Vector3 Vector3::operator*(const float s) const { return Multiply(s); }
inline Vector3 operator*(float s, const Vector3& v) { return v * s; }
inline Vector3 Vector3::operator/(const float s) const { return Multiply(1.0f / s); }

inline Vector3 Vector3::operator-() const { return { -x, -y, -z }; }
inline Vector3 Vector3::operator+() const { return { x, y, z }; }

inline Vector3& Vector3::operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
inline Vector3& Vector3::operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
inline Vector3& Vector3::operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
inline Vector3& Vector3::operator/=(float s) { x /= s; y /= s; z /= s; return *this; }


/// <summary>
/// Vector3の加法
/// </summary>
inline Vector3 Vector3::Add(const Vector3& a) const {
	Vector3 result{};
	result.x = x + a.x;
	result.y = y + a.y;
	result.z = z + a.z;
	return result;
}

/// <summary>
/// Vector3の減法
/// </summary>
inline Vector3 Vector3::Subtract(const Vector3& a) const {
	Vector3 result{};
	result.x = x - a.x;
	result.y = y - a.y;
	result.z = z - a.z;
	return result;
}

/// <summary>
/// Vector3の積
/// </summary>
inline Vector3 Vector3::Multiply(const Vector3& a) const {
	Vector3 result{};
	result.x = x * a.x;
	result.y = y * a.y;
	result.z = z * a.z;
	return result;
}

/// <summary>
/// Vector3のスカラー倍
/// </summary>
inline Vector3 Vector3::Multiply(const float s) const {
	Vector3 result{};
	result.x = x * s;
	result.y = y * s;
	result.z = z * s;
	return result;
}

/// <summary>
/// ノルム
/// </summary>
inline float Vector3::Norm() const {
	float result = sqrtf(x * x + y * y + z * z);
	return result;
}

/// <summary>
/// 正規化
/// </summary>
inline Vector3 Vector3::Normalize() const {

	Vector3 result{};

	float length = Norm();

	if (length != 0.0f) {
		result.x = x / length;
		result.y = y / length;
		result.z = z / length;
	}

	return result;
}

/// <summary>
/// 内積
/// </summary>
inline float Vector3::Dot(const Vector3 v1) const {
	float result{};
	result = (x * v1.x) + (y * v1.y) + (z * v1.z);
	return result;
}

/// <summary>
/// クロス積
/// </summary>
inline Vector3 Vector3::Cross(const Vector3& v1) const {
	Vector3 result{};
	result.x = (y * v1.z) - (z * v1.y);
	result.y = (z * v1.x) - (x * v1.z);
	result.z = (x * v1.y) - (y * v1.x);
	return result;
}

