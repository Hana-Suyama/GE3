#pragma once

/// <summary>
/// 2x2行列
/// </summary>
struct Matrix2x2 {

	/* --------- 本体 --------- */

	float m[2][2];

	/* --------- 演算子オーバーロード --------- */

	Matrix2x2 operator+(const Matrix2x2& m1) const;
	Matrix2x2 operator-(const Matrix2x2& m1) const;
	Matrix2x2 operator*(const Matrix2x2& m1) const;

	/* --------- 基本的な演算 --------- */

	/// <summary>
	/// 行列の加法
	/// </summary>
	Matrix2x2 Add(const Matrix2x2& matrix1) const;

	/// <summary>
	/// 行列の減法
	/// </summary>
	Matrix2x2 Subtract(const Matrix2x2& matrix1) const;

	/// <summary>
	/// 行列の積
	/// </summary>
	Matrix2x2 Multiply(const Matrix2x2& matrix1) const;

	/* --------- 生成 --------- */

	/// <summary>
	/// 逆行列
	/// </summary>
	Matrix2x2 Inverse() const;

	/// <summary>
	/// 転置行列
	/// </summary>
	Matrix2x2 Transpose() const;

	/// <summary>
	/// 単位行列
	/// </summary>
	static Matrix2x2 MakeIdentity4x4();

};

/// <summary>
/// 演算子オーバーロード
/// </summary>
inline Matrix2x2 Matrix2x2::operator+(const Matrix2x2& m1) const { return Add(m1); }
inline Matrix2x2 Matrix2x2::operator-(const Matrix2x2& m1) const { return Subtract(m1); }
inline Matrix2x2 Matrix2x2::operator*(const Matrix2x2& m1) const { return Multiply(m1); }

/// <summary>
/// 行列の加法
/// </summary>
inline Matrix2x2 Matrix2x2::Add(const Matrix2x2& matrix1) const {
	Matrix2x2 Return{};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			Return.m[i][j] = m[i][j] + matrix1.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の減法
/// </summary>
inline Matrix2x2 Matrix2x2::Subtract(const Matrix2x2& matrix1) const {
	Matrix2x2 Return{};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			Return.m[i][j] = m[i][j] - matrix1.m[i][j];
		}
	}
	return Return;
}

/// <summary>
/// 行列の積
/// </summary>
inline Matrix2x2 Matrix2x2::Multiply(const Matrix2x2& matrix1) const {
	Matrix2x2 Return{};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Return.m[i][j] += m[i][k] * matrix1.m[k][j];
			}
		}
	}
	return Return;
}

/// <summary>
/// 逆行列
/// </summary>
inline Matrix2x2 Matrix2x2::Inverse() const {
	Matrix2x2 Return{};
	float A = 0;

	A = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);

	Return.m[0][0] =
		A * m[1][1];

	Return.m[0][1] =
		A * -m[0][1];

	Return.m[1][0] =
		A * -m[1][0];

	Return.m[1][1] =
		A * m[0][0];

	return Return;
}

/// <summary>
/// 転置行列
/// </summary>
inline Matrix2x2 Matrix2x2::Transpose() const {
	Matrix2x2 Return{};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			Return.m[i][j] = m[j][i];
		}
	}
	return Return;
}

/// <summary>
/// 単位行列
/// </summary>
inline Matrix2x2 Matrix2x2::MakeIdentity4x4() {
	Matrix2x2 Return{};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (i == j) {
				Return.m[i][j] = 1;
			}
		}
	}
	return Return;
}