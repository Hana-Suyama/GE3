#pragma once
#include "cmath"
#include "Vector3.h"

/// <summary>
/// クォータニオン
/// </summary>
struct Quaternion {

	/* --------- 本体 --------- */

	float x;
	float y;
	float z;
	float w;

	/* --------- 演算子オーバーロード --------- */

	Quaternion operator+(const Quaternion& v1) const;
	Quaternion operator-(const Quaternion& v1) const;
	Quaternion operator*(const float s) const;
	Quaternion operator/(const float s) const;

	Quaternion operator-() const;
	Quaternion operator+() const;

	Quaternion& operator*=(const float s);
	Quaternion& operator-=(const Quaternion& v);
	Quaternion& operator+=(const Quaternion& v);
	Quaternion& operator/=(const float s);

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// Quaternionの加法
	/// </summary>
	Quaternion Add(const Quaternion& a) const;

	/// <summary>
	/// Quaternionの減法
	/// </summary>
	Quaternion Subtract(const Quaternion& a) const;

	/// <summary>
	/// Quaternionの積
	/// </summary>
	Quaternion Multiply(const Quaternion& rhs) const;

	/// <summary>
	/// Quaternionのスカラー倍
	/// </summary>
	Quaternion Multiply(const float s) const;

	/* --------- 数学 --------- */

	/// <summary>
	/// 内積
	/// </summary>
	/// <param name="v1"></param>
	/// <param name="v2"></param>
	/// <returns></returns>
	float Dot(const Quaternion v1) const;

	/// <summary>
	/// 単位クォータニオン
	/// </summary>
	/// <returns></returns>
	Quaternion IdentityQuaternion() const;

	/// <summary>
	/// 共役
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Quaternion Conjugate() const;

	/// <summary>
	/// ノルム
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	float Norm() const;

	/// <summary>
	/// 正規化
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Quaternion Normalize() const;

	/// <summary>
	/// 逆クォータニオン
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Quaternion Inverse() const;

	/// <summary>
	/// 虚部(ベクトル部)
	/// </summary>
	/// <returns></returns>
	Vector3 ToVector3() const;

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Quaternion Quaternion::operator+(const Quaternion& v1) const { return Add(v1); }
inline Quaternion Quaternion::operator-(const Quaternion& v1) const { return Subtract(v1); }
inline Quaternion Quaternion::operator*(const float s) const { return Multiply(s); }
inline Quaternion operator*(float s, const Quaternion& v) { return v * s; }
inline Quaternion Quaternion::operator/(const float s) const { return Multiply(1.0f / s); }

inline Quaternion Quaternion::operator-() const { return { -x, -y, -z, -w }; }
inline Quaternion Quaternion::operator+() const { return { x, y, z, w }; }

inline Quaternion& Quaternion::operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
inline Quaternion& Quaternion::operator-=(const Quaternion& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
inline Quaternion& Quaternion::operator+=(const Quaternion& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
inline Quaternion& Quaternion::operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

/// <summary>
/// Quaternionの加法
/// </summary>
inline Quaternion Quaternion::Add(const Quaternion& a) const {
	Quaternion result{};
	result.x = x + a.x;
	result.y = y + a.y;
	result.z = z + a.z;
	result.w = w + a.w;
	return result;
}

/// <summary>
/// Quaternionの減法
/// </summary>
inline Quaternion Quaternion::Subtract(const Quaternion& a) const {
	Quaternion result{};
	result.x = x - a.x;
	result.y = y - a.y;
	result.z = z - a.z;
	result.w = w - a.w;
	return result;
}

/// <summary>
/// Quaternionの積
/// </summary>
inline Quaternion Quaternion::Multiply(const Quaternion& rhs) const {
	Quaternion r{};

	Vector3 qv = { x, y, z };
	Vector3 rv = { rhs.x, rhs.y, rhs.z };

	r.w = (w * rhs.w) - qv.Dot(rv);
	Vector3 result = qv.Cross(rv) + (rhs.w * qv) + (w * rv);

	r.x = result.x;
	r.y = result.y;
	r.z = result.z;

	return r;
}

/// <summary>
/// Quaternionのスカラー倍
/// </summary>
inline Quaternion Quaternion::Multiply(const float s) const {
	Quaternion result{};
	result.x = x * s;
	result.y = y * s;
	result.z = z * s;
	result.w = w * s;
	return result;
}

/// <summary>
/// 内積
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
inline float Quaternion::Dot(const Quaternion v1) const {
	float result{};
	result = (x * v1.x) + (y * v1.y) + (z * v1.z) + (w * v1.w);
	return result;
}

/// <summary>
/// 単位クォータニオン
/// </summary>
/// <returns></returns>
inline Quaternion Quaternion::IdentityQuaternion() const {
	Quaternion r = { 0.0f, 0.0f, 0.0f, 1.0f };
	return r;
}

/// <summary>
/// 共役
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
inline Quaternion Quaternion::Conjugate() const {
	Quaternion r{};

	r.x = -x;
	r.y = -y;
	r.z = -z;
	r.w = w;

	return r;
}

/// <summary>
/// ノルム
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
inline float Quaternion::Norm() const {
	float r = sqrtf(w * w + x * x + y * y + z * z);
	return r;
}

/// <summary>
/// 正規化
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
inline Quaternion Quaternion::Normalize() const {
	Quaternion result{};

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
/// 逆クォータニオン
/// </summary>
/// <param name="quaternion"></param>
/// <returns></returns>
inline Quaternion Quaternion::Inverse() const {
	Quaternion r{};

	r.x = Conjugate().x / (Norm() * Norm());
	r.y = Conjugate().y / (Norm() * Norm());
	r.z = Conjugate().z / (Norm() * Norm());
	r.w = Conjugate().w / (Norm() * Norm());

	return r;
}

/// <summary>
/// 虚部(ベクトル部)
/// </summary>
/// <returns></returns>
inline Vector3 Quaternion::ToVector3() const {
	Vector3 result{};

	result.x = x;
	result.y = y;
	result.z = z;

	return result;
}