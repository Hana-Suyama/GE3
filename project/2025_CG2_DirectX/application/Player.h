#pragma once
#include "../engine/Object3D/Object3DBasic.h"
#include "../engine/Model/ModelManager.h"
#include "../engine/Object3D/Object3D.h"
#include "../engine/Input/input.h"
#include "MapChipField.h"
class Player
{
public:

	struct CollisionMapInfo {
		bool isCeilingCollision = false;
		bool isGroundCollision = false;
		bool isWallCollision = false;
		Vector3 velocity;
	};

	enum Corner {
		kRightBottom,	//右下
		kLeftBottom,	//左下
		kRightTop,		//右上
		kLeftTop,		//左上

		kNumCorner		//要素数

	};

	enum class Behavior {
		kRoot,	//通常攻撃
		kAttack,	//攻撃中
		kUnknown,
	};

	//攻撃フェーズ(型)
	enum class AttackPhase {
		kChage,	//溜め
		kRush,	//突進
		kInterval,	//余韻
	};

public:

	void Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Object3D* model, Camera* camera);

	void Update();

	void Draw();

	void Respawn(const Vector3& position);

	void BounceFromEnemy();

	void Move();

	void MapCollisionCheck(CollisionMapInfo& info);

	void MapCollisionCheckUp(CollisionMapInfo& info);
	void MapCollisionCheckDown(CollisionMapInfo& info);
	void MapCollisionCheckRight(CollisionMapInfo& info);
	void MapCollisionCheckLeft(CollisionMapInfo& info);

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	void CheckResultMove(const CollisionMapInfo& info);

	void CeilingCollisionMove(const CollisionMapInfo& info);

	void OnGroundSwitch(const CollisionMapInfo& info);

	void isWallHit(const CollisionMapInfo& info);

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; };

	const Vector3& GetWorldTransform() { return model_->GetTransform().translate; };

	const Vector3& GetVelocity() const { return velocity_; };




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

	Camera* camera_ = nullptr;

	//加速度
	Vector3 velocity_ = {};

	//速度
	static inline const float kAcceleration = 0.03f;
	//速度減衰率
	static inline const float kAttenuation = 0.22f;
	static inline const float kStopSpeedThreshold = 0.01f;
	//上限速度
	static inline const float kLimitRunSpeed = 0.5f;
	//左右
	LRDirection lrDirection_ = LRDirection::kRight;
	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	//旋回タイマー
	float turnTimer_ = 0.0f;
	//旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	//接地状態フラグ
	bool onGround_ = true;
	float coyoteTimer_ = 0.0f;
	float jumpBufferTimer_ = 0.0f;
	bool isJumpRising_ = false;

	static inline const float kCoyoteTime = 0.10f;
	static inline const float kJumpBufferTime = 0.12f;
	static inline const float kJumpCutMultiplier = 0.5f;
	//重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.03f;
	//最大落下速度(下方向)
	static inline const float kLimitFallSpeed = 1.0f;
	//ジャンプ初速(上方向)
	static inline const float kJumpAcceleration = 0.5f;
	static inline const float kEnemyBounceAcceleration = 0.35f;
	//マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;
	//キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kBlank = 0.1f;
	//着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.25f;
	//壁接触時の速度減衰率
	static inline const float kAttenuationWall = 0.1f;

	//デスフラグ
	bool isDead_ = false;

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

