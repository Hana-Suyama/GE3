#pragma once
#define _USE_MATH_DEFINES
#include "cmath"
#include <algorithm>
#include "MyMath.h"
#include <cassert>
using namespace std;

// void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char a[]) {
//	Novice::ScreenPrintf(x, y, "%s", a);
//	for (int row = 0; row < 4; ++row) {
//		for (int column = 0; column < 4; ++column) {
//			Novice::ScreenPrintf(x + column * kColumnWidth, 20 + y + row * kRowHeight, "%6.02f",
//matrix.m[row][column]);
//		}
//	}
// }
//

Vector3 operator+(const Vector3& v1, const Vector3& v2) { return Add(v1, v2); }
Vector3 operator-(const Vector3& v1, const Vector3& v2) { return Subtract(v1, v2); }
Vector3 operator*(float s, const Vector3& v) { return Multiply(v, s); }
Vector3 operator*(const Vector3& v, float s) { return s * v; }
Vector3 operator/(const Vector3& v, float s) { return Multiply(v, 1.0f / s); }
Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) { return Add(m1, m2); }
Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) { return Subtract(m1, m2); }
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return Multiply(m1, m2); }

Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }
Vector3 operator+(const Vector3& v) { return v; }

/// <summary>
/// 角度をラジアンに変換
/// </summary>
float DEGtoRAD(float degree) {
	float result;
	result = degree * ((float)M_PI / 180);
	return result;
}

/// <summary>
/// 行列の加法
/// </summary>
Matrix4x4 Add(Matrix4x4 matrix1, Matrix4x4 matrix2) {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = matrix1.m[i][j] + matrix2.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の減法
/// </summary>
Matrix4x4 Subtract(Matrix4x4 matrix1, Matrix4x4 matrix2) {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = matrix1.m[i][j] - matrix2.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の積
/// </summary>
Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2) {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				Return.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}
	return Return;
}

/// <summary>
/// Vector3の加法
/// </summary>
Vector3 Add(Vector3 a, Vector3 b) {
	Vector3 result{};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

/// <summary>
/// Vector3の減法
/// </summary>
Vector3 Subtract(Vector3 a, Vector3 b) {
	Vector3 result{};
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

/// <summary>
/// Vector3の積
/// </summary>
Vector3 Multiply(const Vector3 a, const Vector3 b) {
	Vector3 result{};
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

/// <summary>
/// Vector3のスカラー倍
/// </summary>
Vector3 Multiply(const Vector3& a, const float& b) {
	Vector3 result{};
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

/// <summary>
/// ベクトルと行列の積
/// </summary>
Vector3 Multiply(Vector3 vector, Matrix4x4 matrix) {
	Vector3 Return = {};
	Return.x =
		(vector.x * matrix.m[0][0]) + (vector.y * matrix.m[1][0]) + (vector.z * matrix.m[2][0]) + (matrix.m[3][0]);
	Return.y =
		(vector.x * matrix.m[0][1]) + (vector.y * matrix.m[1][1]) + (vector.z * matrix.m[2][1]) + (matrix.m[3][1]);
	Return.z =
		(vector.x * matrix.m[0][2]) + (vector.y * matrix.m[1][2]) + (vector.z * matrix.m[2][2]) + (matrix.m[3][2]);
	return Return;
}

/// <summary>
/// 正規化
/// </summary>
Vector3 Normalize(const Vector3& a) {

	Vector3 result{};

	float length = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);

	if (length != 0.0f) {
		result.x = a.x / length;
		result.y = a.y / length;
		result.z = a.z / length;
	}

	return result;
}

/// <summary>
/// 内積
/// </summary>
float Dot(const Vector3 v1, const Vector3 v2) {
	float Result{};
	Result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return Result;
}

/// <summary>
/// クロス積
/// </summary>
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = (v1.y * v2.z) - (v1.z * v2.y);
	result.y = (v1.z * v2.x) - (v1.x * v2.z);
	result.z = (v1.x * v2.y) - (v1.y * v2.x);
	return result;
}

/// <summary>
/// 逆行列
/// </summary>
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 Return{};
	float A = 0;

	A = (m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]) +
	    (m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]) +
	    (m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]) -
	    (m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]) -
	    (m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]) -
	    (m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]) -
	    (m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]) -
	    (m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]) -
	    (m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]) +
	    (m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]) +
	    (m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]) +
	    (m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]) +
	    (m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]) +
	    (m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]) +
	    (m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]) -
	    (m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]) -
	    (m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]) -
	    (m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]) -
	    (m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]) -
	    (m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]) -
	    (m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]) +
	    (m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]) +
	    (m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]) +
	    (m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0]);

	Return.m[0][0] =
	    (1 / A) * ((m.m[1][1] * m.m[2][2] * m.m[3][3]) + (m.m[1][2] * m.m[2][3] * m.m[3][1]) +
	               (m.m[1][3] * m.m[2][1] * m.m[3][2]) - (m.m[1][3] * m.m[2][2] * m.m[3][1]) -
	               (m.m[1][2] * m.m[2][1] * m.m[3][3]) - (m.m[1][1] * m.m[2][3] * m.m[3][2]));

	Return.m[0][1] =
	    (1 / A) * (-(m.m[0][1] * m.m[2][2] * m.m[3][3]) - (m.m[0][2] * m.m[2][3] * m.m[3][1]) -
	               (m.m[0][3] * m.m[2][1] * m.m[3][2]) + (m.m[0][3] * m.m[2][2] * m.m[3][1]) +
	               (m.m[0][2] * m.m[2][1] * m.m[3][3]) + (m.m[0][1] * m.m[2][3] * m.m[3][2]));

	Return.m[0][2] =
	    (1 / A) * ((m.m[0][1] * m.m[1][2] * m.m[3][3]) + (m.m[0][2] * m.m[1][3] * m.m[3][1]) +
	               (m.m[0][3] * m.m[1][1] * m.m[3][2]) - (m.m[0][3] * m.m[1][2] * m.m[3][1]) -
	               (m.m[0][2] * m.m[1][1] * m.m[3][3]) - (m.m[0][1] * m.m[1][3] * m.m[3][2]));

	Return.m[0][3] =
	    (1 / A) * (-(m.m[0][1] * m.m[1][2] * m.m[2][3]) - (m.m[0][2] * m.m[1][3] * m.m[2][1]) -
	               (m.m[0][3] * m.m[1][1] * m.m[2][2]) + (m.m[0][3] * m.m[1][2] * m.m[2][1]) +
	               (m.m[0][2] * m.m[1][1] * m.m[2][3]) + (m.m[0][1] * m.m[1][3] * m.m[2][2]));

	Return.m[1][0] =
	    (1 / A) * (-(m.m[1][0] * m.m[2][2] * m.m[3][3]) - (m.m[1][2] * m.m[2][3] * m.m[3][0]) -
	               (m.m[1][3] * m.m[2][0] * m.m[3][2]) + (m.m[1][3] * m.m[2][2] * m.m[3][0]) +
	               (m.m[1][2] * m.m[2][0] * m.m[3][3]) + (m.m[1][0] * m.m[2][3] * m.m[3][2]));

	Return.m[1][1] =
	    (1 / A) * ((m.m[0][0] * m.m[2][2] * m.m[3][3]) + (m.m[0][2] * m.m[2][3] * m.m[3][0]) +
	               (m.m[0][3] * m.m[2][0] * m.m[3][2]) - (m.m[0][3] * m.m[2][2] * m.m[3][0]) -
	               (m.m[0][2] * m.m[2][0] * m.m[3][3]) - (m.m[0][0] * m.m[2][3] * m.m[3][2]));

	Return.m[1][2] =
	    (1 / A) * (-(m.m[0][0] * m.m[1][2] * m.m[3][3]) - (m.m[0][2] * m.m[1][3] * m.m[3][0]) -
	               (m.m[0][3] * m.m[1][0] * m.m[3][2]) + (m.m[0][3] * m.m[1][2] * m.m[3][0]) +
	               (m.m[0][2] * m.m[1][0] * m.m[3][3]) + (m.m[0][0] * m.m[1][3] * m.m[3][2]));

	Return.m[1][3] =
	    (1 / A) * ((m.m[0][0] * m.m[1][2] * m.m[2][3]) + (m.m[0][2] * m.m[1][3] * m.m[2][0]) +
	               (m.m[0][3] * m.m[1][0] * m.m[2][2]) - (m.m[0][3] * m.m[1][2] * m.m[2][0]) -
	               (m.m[0][2] * m.m[1][0] * m.m[2][3]) - (m.m[0][0] * m.m[1][3] * m.m[2][2]));

	Return.m[2][0] =
	    (1 / A) * ((m.m[1][0] * m.m[2][1] * m.m[3][3]) + (m.m[1][1] * m.m[2][3] * m.m[3][0]) +
	               (m.m[1][3] * m.m[2][0] * m.m[3][1]) - (m.m[1][3] * m.m[2][1] * m.m[3][0]) -
	               (m.m[1][1] * m.m[2][0] * m.m[3][3]) - (m.m[1][0] * m.m[2][3] * m.m[3][1]));

	Return.m[2][1] =
	    (1 / A) * (-(m.m[0][0] * m.m[2][1] * m.m[3][3]) - (m.m[0][1] * m.m[2][3] * m.m[3][0]) -
	               (m.m[0][3] * m.m[2][0] * m.m[3][1]) + (m.m[0][3] * m.m[2][1] * m.m[3][0]) +
	               (m.m[0][1] * m.m[2][0] * m.m[3][3]) + (m.m[0][0] * m.m[2][3] * m.m[3][1]));

	Return.m[2][2] =
	    (1 / A) * ((m.m[0][0] * m.m[1][1] * m.m[3][3]) + (m.m[0][1] * m.m[1][3] * m.m[3][0]) +
	               (m.m[0][3] * m.m[1][0] * m.m[3][1]) - (m.m[0][3] * m.m[1][1] * m.m[3][0]) -
	               (m.m[0][1] * m.m[1][0] * m.m[3][3]) - (m.m[0][0] * m.m[1][3] * m.m[3][1]));

	Return.m[2][3] =
	    (1 / A) * (-(m.m[0][0] * m.m[1][1] * m.m[2][3]) - (m.m[0][1] * m.m[1][3] * m.m[2][0]) -
	               (m.m[0][3] * m.m[1][0] * m.m[2][1]) + (m.m[0][3] * m.m[1][1] * m.m[2][0]) +
	               (m.m[0][1] * m.m[1][0] * m.m[2][3]) + (m.m[0][0] * m.m[1][3] * m.m[2][1]));

	Return.m[3][0] =
	    (1 / A) * (-(m.m[1][0] * m.m[2][1] * m.m[3][2]) - (m.m[1][1] * m.m[2][2] * m.m[3][0]) -
	               (m.m[1][2] * m.m[2][0] * m.m[3][1]) + (m.m[1][2] * m.m[2][1] * m.m[3][0]) +
	               (m.m[1][1] * m.m[2][0] * m.m[3][2]) + (m.m[1][0] * m.m[2][2] * m.m[3][1]));

	Return.m[3][1] =
	    (1 / A) * ((m.m[0][0] * m.m[2][1] * m.m[3][2]) + (m.m[0][1] * m.m[2][2] * m.m[3][0]) +
	               (m.m[0][2] * m.m[2][0] * m.m[3][1]) - (m.m[0][2] * m.m[2][1] * m.m[3][0]) -
	               (m.m[0][1] * m.m[2][0] * m.m[3][2]) - (m.m[0][0] * m.m[2][2] * m.m[3][1]));

	Return.m[3][2] =
	    (1 / A) * (-(m.m[0][0] * m.m[1][1] * m.m[3][2]) - (m.m[0][1] * m.m[1][2] * m.m[3][0]) -
	               (m.m[0][2] * m.m[1][0] * m.m[3][1]) + (m.m[0][2] * m.m[1][1] * m.m[3][0]) +
	               (m.m[0][1] * m.m[1][0] * m.m[3][2]) + (m.m[0][0] * m.m[1][2] * m.m[3][1]));

	Return.m[3][3] =
	    (1 / A) * ((m.m[0][0] * m.m[1][1] * m.m[2][2]) + (m.m[0][1] * m.m[1][2] * m.m[2][0]) +
	               (m.m[0][2] * m.m[1][0] * m.m[2][1]) - (m.m[0][2] * m.m[1][1] * m.m[2][0]) -
	               (m.m[0][1] * m.m[1][0] * m.m[2][2]) - (m.m[0][0] * m.m[1][2] * m.m[2][1]));

	return Return;
}

/// <summary>
/// 転置行列
/// </summary>
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = m.m[j][i];
		}
	}
	return Return;
}

/// <summary>
/// 単位行列
/// </summary>
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				Return.m[i][j] = 1;
			}
		}
	}
	return Return;
}

/// <summary>
/// 平行移動行列
/// </summary>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				Return.m[i][j] = 1;
			}
		}
	}
	Return.m[3][0] = translate.x;
	Return.m[3][1] = translate.y;
	Return.m[3][2] = translate.z;
	return Return;
}

/// <summary>
/// 拡大縮小行列
/// </summary>
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 Return{};
	Return.m[0][0] = scale.x;
	Return.m[1][1] = scale.y;
	Return.m[2][2] = scale.z;
	Return.m[3][3] = 1;
	return Return;
}

/// <summary>
/// 座標変換
/// </summary>
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 Return{};
	Return.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] +
	           1.0f * matrix.m[3][0];
	Return.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] +
	           1.0f * matrix.m[3][1];
	Return.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] +
	           1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] +
	          1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	Return.x /= w;
	Return.y /= w;
	Return.z /= w;
	return Return;
}

/// <summary>
/// X軸回転行列
/// </summary>
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 Return{};
	Return.m[0][0] = 1;
	Return.m[1][1] = std::cos(radian);
	Return.m[1][2] = std::sin(radian);
	Return.m[2][1] = -std::sin(radian);
	Return.m[2][2] = std::cos(radian);
	Return.m[3][3] = 1;
	return Return;
}

/// <summary>
/// Y軸回転行列
/// </summary>
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 Return{};
	Return.m[0][0] = std::cos(radian);
	Return.m[0][2] = -std::sin(radian);
	Return.m[1][1] = 1;
	Return.m[2][0] = std::sin(radian);
	Return.m[2][2] = std::cos(radian);
	Return.m[3][3] = 1;
	return Return;
}

/// <summary>
/// Z軸回転行列
/// </summary>
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 Return{};
	Return.m[0][0] = std::cos(radian);
	Return.m[0][1] = std::sin(radian);
	Return.m[1][0] = -std::sin(radian);
	Return.m[1][1] = std::cos(radian);
	Return.m[2][2] = 1;
	Return.m[3][3] = 1;
	return Return;
}

/// <summary>
/// 3次元アフィン変換行列
/// </summary>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result{};
	result = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));
	result = Multiply(result, rotateXYZMatrix);
	result = Multiply(result, MakeTranslateMatrix(translate));
	return result;
}

/// <summary>
/// 透視投影行列
/// </summary>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result{};
	result.m[0][0] = (1 / aspectRatio) * (1 / std::tan(fovY / 2));
	result.m[1][1] = (1 / std::tan(fovY / 2));
	result.m[2][2] = (farClip / (farClip - nearClip));
	result.m[2][3] = 1;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	return result;
}

/// <summary>
/// 正射影行列
/// </summary>
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{};
	result.m[0][0] = 2 / (right - left);
	result.m[1][1] = 2 / (top - bottom);
	result.m[2][2] = 1 / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1;
	return result;
}

/// <summary>
/// ビューポート変換行列
/// </summary>
Matrix4x4 MakeViewportMatrix(
    float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result{};
	result.m[0][0] = width / 2;
	result.m[1][1] = -(height / 2);
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2);
	result.m[3][1] = top + (height / 2);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1;
	return result;
}

/// <summary>
/// ベクトル変換
/// </summary>
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
	Vector3 result{
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] };
	return result;
}

/// <summary>
/// 正射影ベクトル
/// </summary>
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	Vector3 Result{};
	Result = Multiply(Normalize(v2), Dot(v1, Normalize(v2)));
	return Result;
}

/// <summary>
/// 最近接点
/// </summary>
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 Result{};
	Result = Add(segment.origin, Project(Subtract(point, segment.origin), segment.diff));
	return Result;
}

/// <summary>
/// 3次元の距離の算出
/// </summary>
float Length(Vector3 A, Vector3 B) {
	float a = B.x - A.x;
	float b = B.y - A.y;
	float c = B.z - A.z;
	float d = sqrtf(a * a + b * b + c * c);
	return d;
}

/// <summary>
/// 垂直なベクトルを求める
/// </summary>
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f };
	}
	return{ 0.0f, -vector.z, vector.y };
}

/// <summary>
/// 反射ベクトルを求める
/// </summary>
Vector3 Reflect(const Vector3& input, const Vector3& normal) {
	Vector3 result;
	result = input - ((2.0f * Dot(input, normal)) * normal);
	return result;
}

/// <summary>
/// 線形補間
/// </summary>
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 result;
	result.x = (t * v1.x) + ((1.0f - t) * v2.x);
	result.y = (t * v1.y) + ((1.0f - t) * v2.y);
	result.z = (t * v1.z) + ((1.0f - t) * v2.z);
	return result;
}

/// <summary>
/// ベジェ曲線
/// </summary>
Vector3 Bezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t) {
	Vector3 p0p1 = Lerp(p0, p1, t);
	Vector3 p1p2 = Lerp(p1, p2, t);
	Vector3 p = Lerp(p0p1, p1p2, t);
	return p;
}

/// <summary>
/// グリッドの描画
/// </summary>
//void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
//	const float kGridHalfWidth = 2.0f;//Gridの半分の幅
//	const uint32_t kSubdivision = 10;//分割数
//	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);//1つ分の長さ
//	Vector3 StartPos{ 0, 0, 0 };
//
//	//奥から手前への線を順々に引いていく
//	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
//		Vector3 worldStart{};
//		Vector3 worldEnd{};
//		worldStart.z = StartPos.z + kGridHalfWidth;
//		worldEnd.z = worldStart.z - kGridHalfWidth * 2.0f;
//		worldStart.x = kGridHalfWidth - (xIndex * kGridEvery);
//		worldEnd.x = worldStart.x;
//
//		Vector3 StartVertex{};
//		Vector3 EndVertex{};
//
//		Vector3 StartndcVertex = Transform(worldStart, viewProjectionMatrix);
//		StartVertex = Transform(StartndcVertex, viewportMatrix);
//		Vector3 EndndcVertex = Transform(worldEnd, viewProjectionMatrix);
//		EndVertex = Transform(EndndcVertex, viewportMatrix);
//
//		if (xIndex == 5) {
//			Novice::DrawLine((int)StartVertex.x, (int)StartVertex.y, (int)EndVertex.x, (int)EndVertex.y, BLACK);
//		} else {
//			Novice::DrawLine((int)StartVertex.x, (int)StartVertex.y, (int)EndVertex.x, (int)EndVertex.y, 0xAAAAAAFF);
//		}
//	}
//
//	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
//		Vector3 worldStart{};
//		Vector3 worldEnd{};
//		worldStart.x = kGridHalfWidth;
//		worldEnd.x = worldStart.x - kGridHalfWidth * 2.0f;
//		worldStart.z = kGridHalfWidth - (zIndex * kGridEvery);
//		worldEnd.z = worldStart.z;
//
//		Vector3 StartVertex{};
//		Vector3 EndVertex{};
//
//		Vector3 StartndcVertex = Transform(worldStart, viewProjectionMatrix);
//		StartVertex = Transform(StartndcVertex, viewportMatrix);
//		Vector3 EndndcVertex = Transform(worldEnd, viewProjectionMatrix);
//		EndVertex = Transform(EndndcVertex, viewportMatrix);
//
//		if (zIndex == 5) {
//			Novice::DrawLine((int)StartVertex.x, (int)StartVertex.y, (int)EndVertex.x, (int)EndVertex.y, BLACK);
//		} else {
//			Novice::DrawLine((int)StartVertex.x, (int)StartVertex.y, (int)EndVertex.x, (int)EndVertex.y, 0xAAAAAAFF);
//		}
//	}
//}

/// <summary>
/// 球の描画
/// </summary>
//void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
//	const uint32_t kSubdivision = 26;//分割数
//	const float kLonEvery = DEGtoRAD(360 / kSubdivision);//経度分割1つ分の角度
//	const float kLatEvery = DEGtoRAD(360 / kSubdivision);//緯度分割1つ分の角度
//	//緯度の方向に分割 0 ~ 2π
//	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
//		float lat = -(float)M_PI / 2.0f + kLatEvery * latIndex;//現在の緯度
//		//経度の方向に分割 0 ~ 2π
//		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
//			float lon = lonIndex * kLonEvery;//現在の経度
//
//			//world座標系でのa, b, cを求める
//			Vector3 a{}, b{}, c{};
//			a.x = sphere.center.x + (cosf(lat) * cosf(lon) * sphere.radius);
//			a.y = sphere.center.y + (sinf(lat) * sphere.radius);
//			a.z = sphere.center.z + (cosf(lat) * sinf(lon) * sphere.radius);
//
//			b.x = sphere.center.x + (cosf(lat + kLatEvery) * cosf(lon) * sphere.radius);
//			b.y = sphere.center.y + (sinf(lat + kLatEvery) * sphere.radius);
//			b.z = sphere.center.z + (cosf(lat + kLatEvery) * sinf(lon) * sphere.radius);
//
//			c.x = sphere.center.x + (cosf(lat) * cosf(lon + kLonEvery) * sphere.radius);
//			c.y = sphere.center.y + (sinf(lat) * sphere.radius);
//			c.z = sphere.center.z + (cosf(lat) * sinf(lon + kLonEvery) * sphere.radius);
//
//			// a,b,cをScreen座標系まで変換
//			Vector3 VertexA{};
//			Vector3 ndcVertexA = Transform(a, viewProjectionMatrix);
//			VertexA = Transform(ndcVertexA, viewportMatrix);
//
//			Vector3 VertexB{};
//			Vector3 ndcVertexB = Transform(b, viewProjectionMatrix);
//			VertexB = Transform(ndcVertexB, viewportMatrix);
//
//			Vector3 VertexC{};
//			Vector3 ndcVertexC = Transform(c, viewProjectionMatrix);
//			VertexC = Transform(ndcVertexC, viewportMatrix);
//
//			//ab,bcで線を引く
//			Novice::DrawLine((int)VertexA.x, (int)VertexA.y, (int)VertexB.x, (int)VertexB.y, color);
//			Novice::DrawLine((int)VertexA.x, (int)VertexA.y, (int)VertexC.x, (int)VertexC.y, color);
//		}
//	}
//}
//
///// <summary>
///// 三角形の描画
///// </summary>
//void DrawLineTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
//	Vector3 screenVertices[3];
//	for (uint32_t i = 0; i < 3; ++i) {
//		Vector3 ndcVertex = Transform(triangle.LocalVertices[i], viewProjectionMatrix);
//		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
//	}
//	Novice::DrawTriangle((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[2].x, (int)screenVertices[2].y, color, kFillModeWireFrame);
//}
//
///// <summary>
///// 平面の描画
///// </summary>
//void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
//	Vector3 center = Multiply(plane.normal, plane.distsnce);
//	Vector3 perpendiculars[4];
//	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
//	perpendiculars[1] = { -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z };
//	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
//	perpendiculars[3] = { -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z };
//
//	Vector3 points[4];
//	for (int32_t index = 0; index < 4; ++index) {
//		Vector3 extend = Multiply(perpendiculars[index], 2.0f);
//		Vector3 point = Add(center, extend);
//		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
//	}
//
//	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
//	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
//	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
//	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
//}
//
///// <summary>
///// AABBの描画
///// </summary>
//void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
//	Vector3 Vertex[8]{};
//	Vector3 screenVertex[8]{};
//	Vertex[0] = { aabb.min.x, aabb.max.y, aabb.max.z };
//	Vertex[1] = aabb.max;
//	Vertex[2] = { aabb.min.x, aabb.min.y, aabb.max.z };
//	Vertex[3] = { aabb.max.x, aabb.min.y, aabb.max.z };
//	Vertex[4] = { aabb.min.x, aabb.max.y, aabb.min.z };
//	Vertex[5] = { aabb.max.x, aabb.max.y, aabb.min.z };
//	Vertex[6] = aabb.min;
//	Vertex[7] = { aabb.max.x, aabb.min.y, aabb.min.z };
//	for (int i = 0; i < 8; i++) {
//		screenVertex[i] = Transform(Transform(Vertex[i], viewProjectionMatrix), viewportMatrix);
//	}
//	
//	Novice::DrawLine((int)screenVertex[0].x, (int)screenVertex[0].y, (int)screenVertex[1].x, (int)screenVertex[1].y, color);
//	Novice::DrawLine((int)screenVertex[0].x, (int)screenVertex[0].y, (int)screenVertex[2].x, (int)screenVertex[2].y, color);
//	Novice::DrawLine((int)screenVertex[1].x, (int)screenVertex[1].y, (int)screenVertex[3].x, (int)screenVertex[3].y, color);
//	Novice::DrawLine((int)screenVertex[2].x, (int)screenVertex[2].y, (int)screenVertex[3].x, (int)screenVertex[3].y, color);
//	Novice::DrawLine((int)screenVertex[0].x, (int)screenVertex[0].y, (int)screenVertex[4].x, (int)screenVertex[4].y, color);
//	Novice::DrawLine((int)screenVertex[1].x, (int)screenVertex[1].y, (int)screenVertex[5].x, (int)screenVertex[5].y, color);
//	Novice::DrawLine((int)screenVertex[2].x, (int)screenVertex[2].y, (int)screenVertex[6].x, (int)screenVertex[6].y, color);
//	Novice::DrawLine((int)screenVertex[3].x, (int)screenVertex[3].y, (int)screenVertex[7].x, (int)screenVertex[7].y, color);
//	Novice::DrawLine((int)screenVertex[4].x, (int)screenVertex[4].y, (int)screenVertex[5].x, (int)screenVertex[5].y, color);
//	Novice::DrawLine((int)screenVertex[4].x, (int)screenVertex[4].y, (int)screenVertex[6].x, (int)screenVertex[6].y, color);
//	Novice::DrawLine((int)screenVertex[6].x, (int)screenVertex[6].y, (int)screenVertex[7].x, (int)screenVertex[7].y, color);
//	Novice::DrawLine((int)screenVertex[5].x, (int)screenVertex[5].y, (int)screenVertex[7].x, (int)screenVertex[7].y, color);
//
//}
//
///// <summary>
///// 2次ベジェ曲線の描画
///// </summary>
//void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
//	const Matrix4x4& viewProjectionMatrix, const Matrix4x4 viewportMatrix, uint32_t color, int divisionNumber) {
//
//	//24分割で描画
//	for (int index = 0; index < divisionNumber; index++) {
//		float t0 = index / float(divisionNumber);
//		float t1 = (index + 1) / float(divisionNumber);
//
//		Vector3 bezier0 = Bezier(controlPoint0, controlPoint1, controlPoint2, t0);
//		Vector3 bezier1 = Bezier(controlPoint0, controlPoint1, controlPoint2, t1);
//
//		Vector3 startVertex = Transform(Transform(bezier0, viewProjectionMatrix), viewportMatrix);
//		Vector3 endVertex = Transform(Transform(bezier1, viewProjectionMatrix), viewportMatrix);
//
//		Novice::DrawLine((int)startVertex.x, (int)startVertex.y, (int)endVertex.x, (int)endVertex.y, color);
//	}
//
//}

/// <summary>
/// 球と平面の衝突判定
/// </summary>
bool IsCollision(const Sphere& sphere, const Plane& plane) {
	float k{};
	k = fabsf(Dot(plane.normal, sphere.center) - plane.distsnce);
	if (k <= sphere.radius) {
		return true;
	} else {
		return false;
	}
}

/// <summary>
/// 線分と平面の衝突判定
/// </summary>
bool IsCollision(const Segment& segment, const Plane& plane) {
	float dot = Dot(plane.normal, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distsnce - Dot(segment.origin, plane.normal)) / dot;

	if (t <= 1 && t >= 0) {
		return true;
	} else {
		return false;
	}
}

/// <summary>
/// 直線と平面の衝突判定
/// </summary>
bool IsCollision(const Line& line, const Plane& plane) {
	float dot = Dot(plane.normal, line.diff);

	if (dot == 0.0f) {
		return false;
	}

	return true;
}

/// <summary>
/// 半直線と平面の衝突判定
/// </summary>
bool IsCollision(const Ray& ray, const Plane& plane) {
	float dot = Dot(plane.normal, ray.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distsnce - Dot(ray.origin, plane.normal)) / dot;

	if (t >= 0) {
		return true;
	} else {
		return false;
	}
}

/// <summary>
/// 三角形と線分の衝突判定
/// </summary>
bool IsCollision(const Triangle& triangle, const Segment& segment) {

	Vector3 v1 = (Subtract(triangle.LocalVertices[1], triangle.LocalVertices[0]));
	Vector3 v2 = (Subtract(triangle.LocalVertices[2], triangle.LocalVertices[1]));
	Vector3 v3 = (Subtract(triangle.LocalVertices[0], triangle.LocalVertices[2]));
	Vector3 n = Normalize(Cross(v1, v2));
	float d = Dot(triangle.LocalVertices[1], n);

	float dot = Dot(n, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (d - Dot(segment.origin, n)) / dot;


	if (t <= 1 && t >= 0) {

		Vector3 p = Add(segment.origin, Multiply(segment.diff, t));
		Vector3 cross01 = Cross(v1, Subtract(p, triangle.LocalVertices[1]));
		Vector3 cross12 = Cross(v2, Subtract(p, triangle.LocalVertices[2]));
		Vector3 cross20 = Cross(v3, Subtract(p, triangle.LocalVertices[0]));

		if (Dot(cross01, n) >= 0.0f &&
			Dot(cross12, n) >= 0.0f &&
			Dot(cross20, n) >= 0.0f) {
			return true;
		}

	}
	return false;
}

/// <summary>
/// AABBとAABBの衝突判定
/// </summary>
bool IsCollision(const AABB& a, const AABB& b) {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) && // x軸
		(a.min.y <= b.max.y && a.max.y >= b.min.y) && // y軸
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true;
	}
	return false;
}

/// <summary>
/// AABBと球の衝突判定
/// </summary>
bool IsCollision(const AABB& aabb, const Sphere& sphere) {
	Vector3 closestPoint{ std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
	std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
	std::clamp(sphere.center.z, aabb.min.z, aabb.max.z) };

	float distance = Length(closestPoint, sphere.center);

	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

/// <summary>
/// AABBと線分の衝突判定
/// </summary>
bool IsCollision(const AABB& aabb, const Segment& segment) {
	//NaNの処理は今は気にしない

	float tminX = (aabb.min.x - segment.origin.x) / segment.diff.x;
	float tminY = (aabb.min.y - segment.origin.y) / segment.diff.y;
	float tminZ = (aabb.min.z - segment.origin.z) / segment.diff.z;
	float tmaxX = (aabb.max.x - segment.origin.x) / segment.diff.x;
	float tmaxY = (aabb.max.y - segment.origin.y) / segment.diff.y;
	float tmaxZ = (aabb.max.z - segment.origin.z) / segment.diff.z;

	float tNearX = min(tminX, tmaxX);
	float tNearY = min(tminY, tmaxY);
	float tNearZ = min(tminZ, tmaxZ);
	float tFarX = max(tminX, tmaxX);
	float tFarY = max(tminY, tmaxY);
	float tFarZ = max(tminZ, tmaxZ);

	float tmin = max(max(tNearX, tNearY), tNearZ);
	float tmax = min(min(tFarX, tFarY), tFarZ);

	//全ての差分ベクトルが0だったらassert
	assert(!(segment.diff.x == 0 && segment.diff.y == 0 && segment.diff.z == 0));

	if (tmin <= tmax) {
		if ((tmin <= 1 && tmin >= 0) || (tmax <= 1 && tmax >= 0)) {
			return true;
		} else if (tmin <= 0 && tmax >= 1) {
			return true;
		}
	}
	return false;
}

/// <summary>
/// AABBと直線の衝突判定
/// </summary>
bool IsCollision(const AABB& aabb, const Line& line) {
	//NaNの処理は今は気にしない

	float tminX = (aabb.min.x - line.origin.x) / line.diff.x;
	float tminY = (aabb.min.y - line.origin.y) / line.diff.y;
	float tminZ = (aabb.min.z - line.origin.z) / line.diff.z;
	float tmaxX = (aabb.max.x - line.origin.x) / line.diff.x;
	float tmaxY = (aabb.max.y - line.origin.y) / line.diff.y;
	float tmaxZ = (aabb.max.z - line.origin.z) / line.diff.z;

	float tNearX = min(tminX, tmaxX);
	float tNearY = min(tminY, tmaxY);
	float tNearZ = min(tminZ, tmaxZ);
	float tFarX = max(tminX, tmaxX);
	float tFarY = max(tminY, tmaxY);
	float tFarZ = max(tminZ, tmaxZ);

	float tmin = max(max(tNearX, tNearY), tNearZ);
	float tmax = min(min(tFarX, tFarY), tFarZ);

	//全ての差分ベクトルが0だったらassert
	assert(!(line.diff.x == 0 && line.diff.y == 0 && line.diff.z == 0));

	if (tmin <= tmax) {
		return true;
	}
	return false;
}

/// <summary>
/// AABBと半直線の衝突判定
/// </summary>
bool IsCollision(const AABB& aabb, const Ray& ray) {
	//NaNの処理は今は気にしない

	float tminX = (aabb.min.x - ray.origin.x) / ray.diff.x;
	float tminY = (aabb.min.y - ray.origin.y) / ray.diff.y;
	float tminZ = (aabb.min.z - ray.origin.z) / ray.diff.z;
	float tmaxX = (aabb.max.x - ray.origin.x) / ray.diff.x;
	float tmaxY = (aabb.max.y - ray.origin.y) / ray.diff.y;
	float tmaxZ = (aabb.max.z - ray.origin.z) / ray.diff.z;

	float tNearX = min(tminX, tmaxX);
	float tNearY = min(tminY, tmaxY);
	float tNearZ = min(tminZ, tmaxZ);
	float tFarX = max(tminX, tmaxX);
	float tFarY = max(tminY, tmaxY);
	float tFarZ = max(tminZ, tmaxZ);

	float tmin = max(max(tNearX, tNearY), tNearZ);
	float tmax = min(min(tFarX, tFarY), tFarZ);

	//全ての差分ベクトルが0だったらassert
	assert(!(ray.diff.x == 0 && ray.diff.y == 0 && ray.diff.z == 0));

	if (tmin <= tmax) {
		if (tmin >= 0 || tmax >= 0) {
			return true;
		}
	}
	return false;
}


/// <summary>
/// カプセルと平面の衝突判定
/// </summary>
bool IsCollision(const Capsule& capsule, const Plane& plane) {
	float dot = Dot(plane.normal, capsule.segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distsnce - Dot(capsule.segment.origin, plane.normal)) / dot;

	if (t <= 1 && t >= 0) {
		return true;
	} else {
		//衝突点を求める
		Vector3 a = capsule.segment.origin + (capsule.segment.diff * t);
		//衝突点と線分上の最近接点を求める
		Vector3 closestPoint = ClosestPoint(a, capsule.segment);
		if (Length(a, closestPoint) <= capsule.radius) {
			true;
		}
		return false;
	}
}

