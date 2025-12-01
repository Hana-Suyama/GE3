#pragma once
#include "../engine/Object3D/Object3DBasic.h"
#include "../engine/Model/ModelManager.h"
#include "../engine/Object3D/Object3D.h"
#include "../engine/input.h"
class Player
{
public:

	void Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Object3D* model, Input* input, Camera* camera);

	void Update();

	void Draw();

	float EaseInOutSine(float t);

	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};


private:

	//左右
	enum class LRDirection {
		kRight,
		kLeft,
	};

	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;

	Object3D* model_ = nullptr;

	Input* input_ = nullptr;

	Camera* camera_ = nullptr;

	//加速度
	Vector3 velocity_ = {};

	//速度
	static inline const float kAcceleration = 0.03f;
	//速度減衰率
	static inline const float kAttenuation = 0.1f;
	//上限速度
	static inline const float kLimitRunSpeed = 0.2f;
	//左右
	LRDirection lrDirection_ = LRDirection::kRight;
	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	//旋回タイマー
	float turnTimer_ = 0.0f;
	//旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;

	//追従対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = { 0, 0, -30.0f };
	//カメラ移動範囲
	Rect movableArea_ = { 0, 100, 0, 100 };
	//カメラの目標座標
	Vector3 targetPositon_{};
	//座標補間割合
	static inline const float kInterpolationRate = 0.3f;
	//速度掛け率
	static inline const float kVelocityBias = 6.0f;
	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMargin = { -600, 600, -320, 320 };
};

