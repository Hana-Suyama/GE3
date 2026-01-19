#pragma once

/// <summary>
/// 3x3行列
/// </summary>
struct Matrix3x3 {

	/* --------- 本体 --------- */

	float m[3][3];

	/* --------- 演算子オーバーロード --------- */

	Matrix3x3 operator+(const Matrix3x3& m1) const;
	Matrix3x3 operator-(const Matrix3x3& m1) const;
	Matrix3x3 operator*(const Matrix3x3& m1) const;

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// 行列の加法
	/// </summary>
	Matrix3x3 Add(const Matrix3x3& matrix1) const;

	/// <summary>
	/// 行列の減法
	/// </summary>
	Matrix3x3 Subtract(const Matrix3x3& matrix1) const;

	/// <summary>
	/// 行列の積
	/// </summary>
	Matrix3x3 Multiply(const Matrix3x3& matrix1) const;

	/* --------- 生成 --------- */

	/// <summary>
	/// 逆行列
	/// </summary>
	Matrix3x3 Inverse() const;

	/// <summary>
	/// 転置行列
	/// </summary>
	Matrix3x3 Transpose() const;

	/// <summary>
	/// 単位行列
	/// </summary>
	static Matrix3x3 MakeIdentity4x4();

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Matrix3x3 Matrix3x3::operator+(const Matrix3x3& m1) const { return Add(m1); }
inline Matrix3x3 Matrix3x3::operator-(const Matrix3x3& m1) const { return Subtract(m1); }
inline Matrix3x3 Matrix3x3::operator*(const Matrix3x3& m1) const { return Multiply(m1); }

/// <summary>
/// 行列の加法
/// </summary>
inline Matrix3x3 Matrix3x3::Add(const Matrix3x3& matrix1) const {
	Matrix3x3 result{};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = m[i][j] + matrix1.m[i][j];
		}
	}
	return result;
}

/// <summary>
/// 行列の減法
/// </summary>
inline Matrix3x3 Matrix3x3::Subtract(const Matrix3x3& matrix1) const {
	Matrix3x3 result{};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = m[i][j] - matrix1.m[i][j];
		}
	}
	return result;
}

/// <summary>
/// 行列の積
/// </summary>
inline Matrix3x3 Matrix3x3::Multiply(const Matrix3x3& matrix1) const {
	Matrix3x3 result{};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				result.m[i][j] += m[i][k] * matrix1.m[k][j];
			}
		}
	}
	return result;
}

/// <summary>
/// 逆行列
/// </summary>
inline Matrix3x3 Matrix3x3::Inverse() const {
	Matrix3x3 result{};
	float A = 0;

	A = (m[0][0] * m[1][1] * m[2][2]) +
		(m[0][1] * m[1][2] * m[2][0]) +
		(m[0][2] * m[1][0] * m[2][1]) -
		(m[0][2] * m[1][1] * m[2][0]) -
		(m[0][1] * m[1][0] * m[2][2]) -
		(m[0][0] * m[1][2] * m[2][1]);

	result.m[0][0] =
		(1 / A) * ((m[1][1] * m[2][2]) - (m[1][2] * m[2][1]));

	result.m[0][1] =
		(1 / A) * ((m[0][2] * m[2][1]) - (m[0][1] * m[2][2]));

	result.m[0][2] =
		(1 / A) * ((m[0][1] * m[1][2]) - (m[0][2] * m[1][1]));

	result.m[1][0] =
		(1 / A) * ((m[1][2] * m[2][0]) - (m[1][0] * m[2][2]));

	result.m[1][1] =
		(1 / A) * ((m[0][0] * m[2][2]) - (m[0][2] * m[2][0]));

	result.m[1][2] =
		(1 / A) * ((m[0][2] * m[1][0]) - (m[0][0] * m[1][2]));

	result.m[2][0] =
		(1 / A) * ((m[1][0] * m[2][1]) - (m[1][1] * m[2][0]));

	result.m[2][1] =
		(1 / A) * (-(m[0][1] * m[2][0]) - (m[0][0] * m[2][1]));

	result.m[2][2] =
		(1 / A) * ((m[0][0] * m[1][1]) - (m[0][1] * m[1][0]));

	return result;
}

/// <summary>
/// 転置行列
/// </summary>
inline Matrix3x3 Matrix3x3::Transpose() const {
	Matrix3x3 result{};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = m[j][i];
		}
	}
	return result;
}

/// <summary>
/// 単位行列
/// </summary>
inline Matrix3x3 Matrix3x3::MakeIdentity4x4() {
	Matrix3x3 result{};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				result.m[i][j] = 1;
			}
		}
	}
	return result;
}