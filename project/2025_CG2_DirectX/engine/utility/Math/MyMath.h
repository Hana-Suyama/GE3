#pragma once
#include <cstdint>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Geometry.h"
#include "Quaternion.h"

namespace MyMath {

	/// <summary>
	/// 角度をラジアンに変換
	/// </summary>
	float DEGtoRAD(float degree);

	/// <summary>
	/// ラジアンを角度に変換
	/// </summary>
	float RADtoDEG(float radian);

	/// <summary>
	/// ベクトルと行列の積
	/// </summary>
	Vector3 Multiply(const Vector3 vector, const Matrix4x4 matrix);

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
	/// 任意軸回転行列
	/// </summary>
	Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);

	/// <summary>
	/// クロス積行列
	/// </summary>
	Matrix4x4 MakeCrossMatrix(const Vector3& vector);

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

	/// <summary>
	/// 方向から方向への回転
	/// </summary>
	/// <param name="from"></param>
	/// <param name="to"></param>
	/// <returns></returns>
	Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);

	/// <summary>
	/// 任意軸回転を表すクォータニオン
	/// </summary>
	/// <param name="axis"></param>
	/// <param name="angle"></param>
	/// <returns></returns>
	Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

	/// <summary>
	/// クォータニオンで回転させたベクトル
	/// </summary>
	/// <param name="vector"></param>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);

	/// <summary>
	/// クォータニオンから回転行列を求める
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

	/// <summary>
	/// 球面線形補間
	/// </summary>
	/// <param name="q0"></param>
	/// <param name="q1"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);
};
