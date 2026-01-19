#pragma once
#include "Vector3.h"

/// <summary>
/// 形状構造体
/// </summary>
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

}

