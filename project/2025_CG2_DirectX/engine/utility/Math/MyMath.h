#pragma once
#include <cstdint>
#include "Vector3.h"
#include "Matrix4x4.h"

namespace MyMath {

	/// <summary>
	/// 球
	/// </summary>
	struct Sphere {
		Vector3 center{};	//!< 中心点
		float radius{};	//!< 半径
	};

	/// <summary>
	/// 直線
	/// </summary>
	struct Line {
		Vector3 origin;	//!<始点
		Vector3 diff;	//!<終点への差分ベクトル
	};

	/// <summary>
	/// 半直線
	/// </summary>
	struct Ray {
		Vector3 origin;	//!<始点
		Vector3 diff;	//!<終点への差分ベクトル
	};

	/// <summary>
	/// 線分
	/// </summary>
	struct Segment {
		Vector3 origin;	//!<始点
		Vector3 diff;	//!<終点への差分ベクトル
	};

	/// <summary>
	/// 平面
	/// </summary>
	struct Plane {
		Vector3 normal;//法線
		float distsnce;//距離
	};

	/// <summary>
	/// 三角形
	/// </summary>
	struct Triangle {
		Vector3 LocalVertices[3];
	};

	/// <summary>
	/// AABB
	/// </summary>
	struct AABB {
		Vector3 min; //!< 最小点
		Vector3 max; //!< 最大点
	};

	/// <summary>
	/// カプセル
	/// </summary>
	struct Capsule {
		Segment segment;
		float radius;
	};

	Vector3 operator+(const Vector3& v1, const Vector3& v2);
	Vector3 operator-(const Vector3& v1, const Vector3& v2);
	Vector3 operator*(float s, const Vector3& v);
	Vector3 operator*(const Vector3& v, float s);
	Vector3 operator/(const Vector3& v, float s);
	Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

	Vector3 operator-(const Vector3& v);
	Vector3 operator+(const Vector3& v);

	/// <summary>
	/// 角度をラジアンに変換
	/// </summary>
	float DEGtoRAD(float degree);

	/// <summary>
	/// 行列の加法
	/// </summary>
	Matrix4x4 Add(Matrix4x4 matrix1, Matrix4x4 matrix2);

	/// <summary>
	/// 行列の減法
	/// </summary>
	Matrix4x4 Subtract(Matrix4x4 matrix1, Matrix4x4 matrix2);

	/// <summary>
	/// 行列の積
	/// </summary>
	Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2);

	/// <summary>
	/// Vector3の加法
	/// </summary>
	Vector3 Add(Vector3 a, Vector3 b);

	/// <summary>
	/// Vector3の減法
	/// </summary>
	Vector3 Subtract(Vector3 a, Vector3 b);

	/// <summary>
	/// Vector3の積
	/// </summary>
	Vector3 Multiply(Vector3 a, Vector3 b);

	/// <summary>
	/// Vector3のスカラー倍
	/// </summary>
	Vector3 Multiply(const Vector3& a, const float& b);

	/// <summary>
	/// ベクトルと行列の積
	/// </summary>
	Vector3 Multiply(const Vector3 vector, const Matrix4x4 matrix);

	/// <summary>
	/// 正規化
	/// </summary>
	Vector3 Normalize(const Vector3& a);

	/// <summary>
	/// 内積
	/// </summary>
	float Dot(const Vector3 v1, const Vector3 v2);

	/// <summary>
	/// クロス積
	/// </summary>
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 逆行列
	/// </summary>
	Matrix4x4 Inverse(const Matrix4x4& m);

	/// <summary>
	/// 転置行列
	/// </summary>
	Matrix4x4 Transpose(const Matrix4x4& m);

	/// <summary>
	/// 単位行列
	/// </summary>
	Matrix4x4 MakeIdentity4x4();

	/// <summary>
	/// 平行移動行列
	/// </summary>
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	/// <summary>
	/// 拡大縮小行列
	/// </summary>
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	/// <summary>
	/// 座標変換
	/// </summary>
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	/// <summary>
	/// X軸回転行列
	/// </summary>
	Matrix4x4 MakeRotateXMatrix(float radian);

	/// <summary>
	/// Y軸回転行列
	/// </summary>
	Matrix4x4 MakeRotateYMatrix(float radian);

	/// <summary>
	/// Z軸回転行列
	/// </summary>
	Matrix4x4 MakeRotateZMatrix(float radian);

	/// <summary>
	/// 3次元アフィン変換行列
	/// </summary>
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	/// <summary>
	/// 透視投影行列
	/// </summary>
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	/// <summary>
	/// 正射影行列
	/// </summary>
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	/// <summary>
	/// ビューポート変換行列
	/// </summary>
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	/// <summary>
	/// ベクトル変換
	/// </summary>
	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	/// <summary>
	/// 正射影ベクトル
	/// </summary>
	Vector3 Project(const Vector3& v1, const Vector3& v2);

	/// <summary>
	/// 最近接点
	/// </summary>
	Vector3 ClosestPoint(const Vector3& point, const Segment& segment);

	/// <summary>
	/// 3次元の距離の算出
	/// </summary>
	float Length(Vector3 A, Vector3 B);

	/// <summary>
	/// 垂直なベクトルを求める
	/// </summary>
	Vector3 Perpendicular(const Vector3& vector);

	/// <summary>
	/// 反射ベクトルを求める
	/// </summary>
	Vector3 Reflect(const Vector3& input, const Vector3& normal);

	/// <summary>
	/// 線形補間
	/// </summary>
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

	/// <summary>
	/// ベジェ曲線
	/// </summary>
	Vector3 Bezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t);

	/// <summary>
	/// グリッドの描画
	/// </summary>
	void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

	/// <summary>
	/// 球の描画
	/// </summary>
	void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

	/// <summary>
	/// 三角形の描画
	/// </summary>
	void DrawLineTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

	/// <summary>
	/// 平面の描画
	/// </summary>
	void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

	/// <summary>
	/// AABBの描画
	/// </summary>
	void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

	/// <summary>
	/// 2次ベジェ曲線の描画
	/// </summary>
	void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
		const Matrix4x4& viewProjectionMatrix, const Matrix4x4 viewportMatrix, uint32_t color, int divisionNumber);

	/// <summary>
	/// 球と平面の衝突判定
	/// </summary>
	bool IsCollision(const Sphere& sphere, const Plane& plane);

	/// <summary>
	/// 線分と平面の衝突判定
	/// </summary>
	bool IsCollision(const Segment& segment, const Plane& plane);

	/// <summary>
	/// 直線と平面の衝突判定
	/// </summary>
	bool IsCollision(const Line& line, const Plane& plane);

	/// <summary>
	/// 半直線と平面の衝突判定
	/// </summary>
	bool IsCollision(const Ray& ray, const Plane& plane);

	/// <summary>
	/// 三角形と線分の衝突判定
	/// </summary>
	bool IsCollision(const Triangle& triangle, const Segment& segment);

	/// <summary>
	/// AABBとAABBの衝突判定
	/// </summary>
	bool IsCollision(const AABB& aabb1, const AABB& aabb2);

	/// <summary>
	/// AABBと球の衝突判定
	/// </summary>
	bool IsCollision(const AABB& aabb, const Sphere& sphere);

	/// <summary>
	/// AABBと線分の衝突判定
	/// </summary>
	bool IsCollision(const AABB& aabb, const Segment& segment);

	/// <summary>
	/// AABBと直線の衝突判定
	/// </summary>
	bool IsCollision(const AABB& aabb, const Line& line);

	/// <summary>
	/// AABBと半直線の衝突判定
	/// </summary>
	bool IsCollision(const AABB& aabb, const Ray& ray);

	/// <summary>
	/// カプセルと平面の衝突判定
	/// </summary>
	bool IsCollision(const Capsule& capsule, const Plane& plane);

};
