#pragma once
#include "cmath"
#include <algorithm>
#include "MyMath.h"
#include <cassert>
#include "Lerp.h"
#include <numbers>

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

namespace MyMath {

	
	/// <summary>
	/// 角度をラジアンに変換
	/// </summary>
	float DEGtoRAD(float degree) {
		float result;
		result = degree * (numbers::pi_v<float> / 180.0f);
		return result;
	}

	/// <summary>
	/// ラジアンを角度に変換
	/// </summary>
	float RADtoDEG(float radian) {
		float result;
		result = radian * (180.0f / numbers::pi_v<float>);
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
	/// 任意軸回転行列
	/// </summary>
	Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle)
	{
		Matrix4x4 Result{};
		Matrix4x4 S = MakeScaleMatrix({ std::cos(angle), std::cos(angle), std::cos(angle) });

		Matrix4x4 a{};
		a.m[0][0] = axis.x;
		a.m[0][1] = axis.y;
		a.m[0][2] = axis.z;

		Matrix4x4 b{};
		b.m[0][0] = axis.x;
		b.m[1][0] = axis.y;
		b.m[2][0] = axis.z;

		Matrix4x4 P = MakeScaleMatrix({ 1.0f - std::cos(angle), 1.0f - std::cos(angle), 1.0f - std::cos(angle) }) * b * a;
		Matrix4x4 C = MakeScaleMatrix({ -std::sin(angle), -std::sin(angle) , -std::sin(angle) }) * MakeCrossMatrix({ axis.x, axis.y, axis.z });
		Result = S + P + C;
		return Result;
	}

	/// <summary>
	/// クロス積行列
	/// </summary>
	Matrix4x4 MakeCrossMatrix(const Vector3& vector)
	{
		Matrix4x4 Result{};

		Result.m[0][0] = 0.0f;
		Result.m[0][1] = -vector.z;
		Result.m[0][2] = vector.y;
		Result.m[1][0] = vector.z;
		Result.m[1][1] = 0.0f;
		Result.m[1][2] = -vector.x;
		Result.m[2][0] = -vector.y;
		Result.m[2][1] = vector.x;
		Result.m[2][2] = 0.0f;

		return Result;
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
		Matrix4x4 rotateXYZMatrix = rotateXMatrix.Multiply(rotateYMatrix.Multiply(rotateZMatrix));
		result = result.Multiply(rotateXYZMatrix);
		result = result.Multiply(MakeTranslateMatrix(translate));
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
		Result = v2.Normalize().Multiply(v1.Dot(v2.Normalize()));
		return Result;
	}

	/// <summary>
	/// 最近接点
	/// </summary>
	Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
		Vector3 Result{};
		Result = segment.origin.Add(Project(point.Subtract(segment.origin), segment.diff));
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
		result = input - ((2.0f * input.Dot(normal)) * normal);
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
		k = fabsf(plane.normal.Dot(sphere.center) - plane.distsnce);
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
		float dot = plane.normal.Dot(segment.diff);

		if (dot == 0.0f) {
			return false;
		}

		float t = (plane.distsnce - segment.origin.Dot(plane.normal)) / dot;

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
		float dot = plane.normal.Dot(line.diff);

		if (dot == 0.0f) {
			return false;
		}

		return true;
	}

	/// <summary>
	/// 半直線と平面の衝突判定
	/// </summary>
	bool IsCollision(const Ray& ray, const Plane& plane) {
		float dot = plane.normal.Dot(ray.diff);

		if (dot == 0.0f) {
			return false;
		}

		float t = (plane.distsnce - ray.origin.Dot(plane.normal)) / dot;

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

		Vector3 v1 = (triangle.LocalVertices[1].Subtract(triangle.LocalVertices[0]));
		Vector3 v2 = (triangle.LocalVertices[2].Subtract(triangle.LocalVertices[1]));
		Vector3 v3 = (triangle.LocalVertices[0].Subtract(triangle.LocalVertices[2]));
		Vector3 n = v1.Cross(v2).Normalize();
		float d = triangle.LocalVertices[1].Dot(n);

		float dot = n.Dot(segment.diff);

		if (dot == 0.0f) {
			return false;
		}

		float t = (d - segment.origin.Dot(n)) / dot;


		if (t <= 1 && t >= 0) {

			Vector3 p = segment.origin.Add(segment.diff.Multiply(t));
			Vector3 cross01 = v1.Cross(p.Subtract(triangle.LocalVertices[1]));
			Vector3 cross12 = v2.Cross(p.Subtract(triangle.LocalVertices[2]));
			Vector3 cross20 = v3.Cross(p.Subtract(triangle.LocalVertices[0]));

			if (cross01.Dot(n) >= 0.0f &&
				cross12.Dot(n) >= 0.0f &&
				cross20.Dot(n) >= 0.0f) {
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
		float dot = plane.normal.Dot(capsule.segment.diff);

		if (dot == 0.0f) {
			return false;
		}

		float t = (plane.distsnce - capsule.segment.origin.Dot(plane.normal)) / dot;

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

	/// <summary>
	/// 方向から方向への回転
	/// </summary>
	/// <param name="from"></param>
	/// <param name="to"></param>
	/// <returns></returns>
	Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to)
	{
		Vector3 n = from.Cross(to).Normalize();
		if (from.x == -to.x && from.y == -to.y && from.z == -to.z) {
			if (from.x != 0.0f || from.y != 0.0f) {
				n = Vector3{ -from.y, -from.x, 0.0f };
			}
		}

		float cos = from.Dot(to);
		Vector3 cross = from.Cross(to);
		float sin = sqrtf(cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);

		Matrix4x4 Result{};
		Matrix4x4 S = MakeScaleMatrix({ cos, cos, cos });

		Matrix4x4 a{};
		a.m[0][0] = n.x;
		a.m[0][1] = n.y;
		a.m[0][2] = n.z;

		Matrix4x4 b{};
		b.m[0][0] = n.x;
		b.m[1][0] = n.y;
		b.m[2][0] = n.z;

		Matrix4x4 P = MakeScaleMatrix({ 1.0f - cos, 1.0f - cos, 1.0f - cos }) * b * a;
		Matrix4x4 C = MakeScaleMatrix({ -sin, -sin , -sin }) * MakeCrossMatrix({ n.x, n.y, n.z });
		Result = S + P + C;
		return Result;
	}

	/// <summary>
	/// 任意軸回転を表すクォータニオン
	/// </summary>
	/// <param name="axis"></param>
	/// <param name="angle"></param>
	/// <returns></returns>
	Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle)
	{
		Quaternion result{};
		Vector3 v = axis * std::sin(angle / 2.0f);

		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		result.w = std::cos(angle / 2.0f);

		return result;

	}

	/// <summary>
	/// クォータニオンで回転させたベクトル
	/// </summary>
	/// <param name="vector"></param>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion)
	{
		Vector3 result{};

		Quaternion r{};
		r.x = vector.x;
		r.y = vector.y;
		r.z = vector.z;
		r.w = 0.0f;

		r = quaternion.Multiply(r).Multiply(quaternion.Conjugate());

		result.x = r.x;
		result.y = r.y;
		result.z = r.z;

		return result;
	}

	/// <summary>
	/// クォータニオンから回転行列を求める
	/// </summary>
	/// <param name="quaternion"></param>
	/// <returns></returns>
	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion)
	{
		Matrix4x4 result{};

		result.m[0][0] = (quaternion.w * quaternion.w) + (quaternion.x * quaternion.x) - (quaternion.y * quaternion.y) - (quaternion.z * quaternion.z);
		result.m[0][1] = 2.0f * ((quaternion.x * quaternion.y) + (quaternion.w * quaternion.z));
		result.m[0][2] = 2.0f * ((quaternion.x * quaternion.z) - (quaternion.w * quaternion.y));
		result.m[0][3] = 0.0f;

		result.m[1][0] = 2.0f * ((quaternion.x * quaternion.y) - (quaternion.w * quaternion.z));
		result.m[1][1] = (quaternion.w * quaternion.w) - (quaternion.x * quaternion.x) + (quaternion.y * quaternion.y) - (quaternion.z * quaternion.z);
		result.m[1][2] = 2.0f * ((quaternion.y * quaternion.z) + (quaternion.w * quaternion.x));
		result.m[1][3] = 0.0f;

		result.m[2][0] = 2.0f * ((quaternion.x * quaternion.z) + (quaternion.w * quaternion.y));
		result.m[2][1] = 2.0f * ((quaternion.y * quaternion.z) - (quaternion.w * quaternion.x));
		result.m[2][2] = (quaternion.w * quaternion.w) - (quaternion.x * quaternion.x) - (quaternion.y * quaternion.y) + (quaternion.z * quaternion.z);
		result.m[2][3] = 0.0f;

		result.m[3][0] = 0.0f;
		result.m[3][1] = 0.0f;
		result.m[3][2] = 0.0f;
		result.m[3][3] = 1.0f;

		return result;
	}

	/// <summary>
	/// 球面線形補間
	/// </summary>
	/// <param name="q0"></param>
	/// <param name="q1"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {

		Quaternion q00 = q0;
		Quaternion q01 = q1;

		float dot = q00.Dot(q01);

		if (dot < 0) {
			q00.x = -q00.x;
			q00.y = -q00.y;
			q00.z = -q00.z;
			q00.w = -q00.w;
			dot = -dot;
		}

		float theta = std::acos(dot);

		float scale0 = std::sin((1.0f - t) * theta) / std::sin(theta);
		float scale1 = std::sin(t * theta) / std::sin(theta);

		Quaternion result{};

		result.x = scale0 * q00.x + scale1 * q01.x;
		result.y = scale0 * q00.y + scale1 * q01.y;
		result.z = scale0 * q00.z + scale1 * q01.z;
		result.w = scale0 * q00.w + scale1 * q01.w;

		return result;
	}

}

