#pragma once

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 {

	/* --------- 本体 --------- */

	float m[4][4];

	/* --------- 演算子オーバーロード --------- */

	Matrix4x4 operator+(const Matrix4x4& m1) const;
	Matrix4x4 operator-(const Matrix4x4& m1) const;
	Matrix4x4 operator*(const Matrix4x4& m1) const;

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// 行列の加法
	/// </summary>
	Matrix4x4 Add(const Matrix4x4& matrix1) const;

	/// <summary>
	/// 行列の減法
	/// </summary>
	Matrix4x4 Subtract(const Matrix4x4& matrix1) const;

	/// <summary>
	/// 行列の積
	/// </summary>
	Matrix4x4 Multiply(const Matrix4x4& matrix1) const;

	/* --------- 生成 --------- */

	/// <summary>
	/// 逆行列
	/// </summary>
	Matrix4x4 Inverse() const;

	/// <summary>
	/// 転置行列
	/// </summary>
	Matrix4x4 Transpose() const;

	/// <summary>
	/// 単位行列
	/// </summary>
	static Matrix4x4 MakeIdentity4x4();

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Matrix4x4 Matrix4x4::operator+(const Matrix4x4& m1) const { return Add(m1); }
inline Matrix4x4 Matrix4x4::operator-(const Matrix4x4& m1) const { return Subtract(m1); }
inline Matrix4x4 Matrix4x4::operator*(const Matrix4x4& m1) const { return Multiply(m1); }

/// <summary>
/// 行列の加法
/// </summary>
inline Matrix4x4 Matrix4x4::Add(const Matrix4x4& matrix1) const {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = m[i][j] + matrix1.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の減法
/// </summary>
inline Matrix4x4 Matrix4x4::Subtract(const Matrix4x4& matrix1) const {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = m[i][j] - matrix1.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の積
/// </summary>
inline Matrix4x4 Matrix4x4::Multiply(const Matrix4x4& matrix1) const {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				Return.m[i][j] += m[i][k] * matrix1.m[k][j];
			}
		}
	}
	return Return;
}

/// <summary>
/// 逆行列
/// </summary>
inline Matrix4x4 Matrix4x4::Inverse() const {
	Matrix4x4 Return{};
	float A = 0;

	A = (m[0][0] * m[1][1] * m[2][2] * m[3][3]) +
		(m[0][0] * m[1][2] * m[2][3] * m[3][1]) +
		(m[0][0] * m[1][3] * m[2][1] * m[3][2]) -
		(m[0][0] * m[1][3] * m[2][2] * m[3][1]) -
		(m[0][0] * m[1][2] * m[2][1] * m[3][3]) -
		(m[0][0] * m[1][1] * m[2][3] * m[3][2]) -
		(m[0][1] * m[1][0] * m[2][2] * m[3][3]) -
		(m[0][2] * m[1][0] * m[2][3] * m[3][1]) -
		(m[0][3] * m[1][0] * m[2][1] * m[3][2]) +
		(m[0][3] * m[1][0] * m[2][2] * m[3][1]) +
		(m[0][2] * m[1][0] * m[2][1] * m[3][3]) +
		(m[0][1] * m[1][0] * m[2][3] * m[3][2]) +
		(m[0][1] * m[1][2] * m[2][0] * m[3][3]) +
		(m[0][2] * m[1][3] * m[2][0] * m[3][1]) +
		(m[0][3] * m[1][1] * m[2][0] * m[3][2]) -
		(m[0][3] * m[1][2] * m[2][0] * m[3][1]) -
		(m[0][2] * m[1][1] * m[2][0] * m[3][3]) -
		(m[0][1] * m[1][3] * m[2][0] * m[3][2]) -
		(m[0][1] * m[1][2] * m[2][3] * m[3][0]) -
		(m[0][2] * m[1][3] * m[2][1] * m[3][0]) -
		(m[0][3] * m[1][1] * m[2][2] * m[3][0]) +
		(m[0][3] * m[1][2] * m[2][1] * m[3][0]) +
		(m[0][2] * m[1][1] * m[2][3] * m[3][0]) +
		(m[0][1] * m[1][3] * m[2][2] * m[3][0]);

	Return.m[0][0] =
		(1 / A) * ((m[1][1] * m[2][2] * m[3][3]) + (m[1][2] * m[2][3] * m[3][1]) +
			(m[1][3] * m[2][1] * m[3][2]) - (m[1][3] * m[2][2] * m[3][1]) -
			(m[1][2] * m[2][1] * m[3][3]) - (m[1][1] * m[2][3] * m[3][2]));

	Return.m[0][1] =
		(1 / A) * (-(m[0][1] * m[2][2] * m[3][3]) - (m[0][2] * m[2][3] * m[3][1]) -
			(m[0][3] * m[2][1] * m[3][2]) + (m[0][3] * m[2][2] * m[3][1]) +
			(m[0][2] * m[2][1] * m[3][3]) + (m[0][1] * m[2][3] * m[3][2]));

	Return.m[0][2] =
		(1 / A) * ((m[0][1] * m[1][2] * m[3][3]) + (m[0][2] * m[1][3] * m[3][1]) +
			(m[0][3] * m[1][1] * m[3][2]) - (m[0][3] * m[1][2] * m[3][1]) -
			(m[0][2] * m[1][1] * m[3][3]) - (m[0][1] * m[1][3] * m[3][2]));

	Return.m[0][3] =
		(1 / A) * (-(m[0][1] * m[1][2] * m[2][3]) - (m[0][2] * m[1][3] * m[2][1]) -
			(m[0][3] * m[1][1] * m[2][2]) + (m[0][3] * m[1][2] * m[2][1]) +
			(m[0][2] * m[1][1] * m[2][3]) + (m[0][1] * m[1][3] * m[2][2]));

	Return.m[1][0] =
		(1 / A) * (-(m[1][0] * m[2][2] * m[3][3]) - (m[1][2] * m[2][3] * m[3][0]) -
			(m[1][3] * m[2][0] * m[3][2]) + (m[1][3] * m[2][2] * m[3][0]) +
			(m[1][2] * m[2][0] * m[3][3]) + (m[1][0] * m[2][3] * m[3][2]));

	Return.m[1][1] =
		(1 / A) * ((m[0][0] * m[2][2] * m[3][3]) + (m[0][2] * m[2][3] * m[3][0]) +
			(m[0][3] * m[2][0] * m[3][2]) - (m[0][3] * m[2][2] * m[3][0]) -
			(m[0][2] * m[2][0] * m[3][3]) - (m[0][0] * m[2][3] * m[3][2]));

	Return.m[1][2] =
		(1 / A) * (-(m[0][0] * m[1][2] * m[3][3]) - (m[0][2] * m[1][3] * m[3][0]) -
			(m[0][3] * m[1][0] * m[3][2]) + (m[0][3] * m[1][2] * m[3][0]) +
			(m[0][2] * m[1][0] * m[3][3]) + (m[0][0] * m[1][3] * m[3][2]));

	Return.m[1][3] =
		(1 / A) * ((m[0][0] * m[1][2] * m[2][3]) + (m[0][2] * m[1][3] * m[2][0]) +
			(m[0][3] * m[1][0] * m[2][2]) - (m[0][3] * m[1][2] * m[2][0]) -
			(m[0][2] * m[1][0] * m[2][3]) - (m[0][0] * m[1][3] * m[2][2]));

	Return.m[2][0] =
		(1 / A) * ((m[1][0] * m[2][1] * m[3][3]) + (m[1][1] * m[2][3] * m[3][0]) +
			(m[1][3] * m[2][0] * m[3][1]) - (m[1][3] * m[2][1] * m[3][0]) -
			(m[1][1] * m[2][0] * m[3][3]) - (m[1][0] * m[2][3] * m[3][1]));

	Return.m[2][1] =
		(1 / A) * (-(m[0][0] * m[2][1] * m[3][3]) - (m[0][1] * m[2][3] * m[3][0]) -
			(m[0][3] * m[2][0] * m[3][1]) + (m[0][3] * m[2][1] * m[3][0]) +
			(m[0][1] * m[2][0] * m[3][3]) + (m[0][0] * m[2][3] * m[3][1]));

	Return.m[2][2] =
		(1 / A) * ((m[0][0] * m[1][1] * m[3][3]) + (m[0][1] * m[1][3] * m[3][0]) +
			(m[0][3] * m[1][0] * m[3][1]) - (m[0][3] * m[1][1] * m[3][0]) -
			(m[0][1] * m[1][0] * m[3][3]) - (m[0][0] * m[1][3] * m[3][1]));

	Return.m[2][3] =
		(1 / A) * (-(m[0][0] * m[1][1] * m[2][3]) - (m[0][1] * m[1][3] * m[2][0]) -
			(m[0][3] * m[1][0] * m[2][1]) + (m[0][3] * m[1][1] * m[2][0]) +
			(m[0][1] * m[1][0] * m[2][3]) + (m[0][0] * m[1][3] * m[2][1]));

	Return.m[3][0] =
		(1 / A) * (-(m[1][0] * m[2][1] * m[3][2]) - (m[1][1] * m[2][2] * m[3][0]) -
			(m[1][2] * m[2][0] * m[3][1]) + (m[1][2] * m[2][1] * m[3][0]) +
			(m[1][1] * m[2][0] * m[3][2]) + (m[1][0] * m[2][2] * m[3][1]));

	Return.m[3][1] =
		(1 / A) * ((m[0][0] * m[2][1] * m[3][2]) + (m[0][1] * m[2][2] * m[3][0]) +
			(m[0][2] * m[2][0] * m[3][1]) - (m[0][2] * m[2][1] * m[3][0]) -
			(m[0][1] * m[2][0] * m[3][2]) - (m[0][0] * m[2][2] * m[3][1]));

	Return.m[3][2] =
		(1 / A) * (-(m[0][0] * m[1][1] * m[3][2]) - (m[0][1] * m[1][2] * m[3][0]) -
			(m[0][2] * m[1][0] * m[3][1]) + (m[0][2] * m[1][1] * m[3][0]) +
			(m[0][1] * m[1][0] * m[3][2]) + (m[0][0] * m[1][2] * m[3][1]));

	Return.m[3][3] =
		(1 / A) * ((m[0][0] * m[1][1] * m[2][2]) + (m[0][1] * m[1][2] * m[2][0]) +
			(m[0][2] * m[1][0] * m[2][1]) - (m[0][2] * m[1][1] * m[2][0]) -
			(m[0][1] * m[1][0] * m[2][2]) - (m[0][0] * m[1][2] * m[2][1]));

	return Return;
}

/// <summary>
/// 転置行列
/// </summary>
inline Matrix4x4 Matrix4x4::Transpose() const {
	Matrix4x4 Return{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			Return.m[i][j] = m[j][i];
		}
	}
	return Return;
}

/// <summary>
/// 単位行列
/// </summary>
inline Matrix4x4 Matrix4x4::MakeIdentity4x4() {
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