#pragma once
#include "Player.h"

//前方宣言
class Player;

struct Rect {
	float left = 0.0f;   // 左端
	float right = 1.0f;  // 右端
	float bottom = 0.0f; // 下端
	float top = 1.0f;    // 上端
};

/// <summary>
/// カメラコントローラ
/// </summary>
class CameraController {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void Reset();

	void SetTarget(Player* target) { target_ = target; };

	void SetMovableArea(Rect area) { movableArea_ = area; };

private:

	//ビュープロジェクション
	Camera* camera_;
	//プレイヤーのポインタ
	Player* target_ = nullptr;
	//追従対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = { 0, 0, -30.0f };
	//カメラ移動範囲
	Rect movableArea_ = { 12, 187, 3, 100 };
	//カメラの目標座標
	Vector3 targetPosition_{};
	//座標補間割合
	static inline const float kInterpolationRate = 0.3f;
	//速度掛け率
	static inline const float kVelocityBias = 6.0f;
	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMargin = { -600, 600, -320, 320 };
};
