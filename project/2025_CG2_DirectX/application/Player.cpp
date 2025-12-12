#include "Player.h"
#include <algorithm>
#include"../engine/utility/Math/MyMath.h"
#include <numbers>
#include <array>
using namespace MyMath;

void Player::Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Object3D* model, Input* input, Camera* camera)
{
	object3dBasic_ = object3dBasic;
	modelManager_ = modelManager;
	model_ = model;
	input_ = input;
	camera_ = camera;

	lrDirection_ = LRDirection::kRight;
	model_->SetRotate({ 0.0f, DEGtoRAD(90.0f), 0.0f });
}

void Player::Update()
{

	Move();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.velocity = velocity_;

	// マップ衝突チェック
	MapCollisionCheck(collisionMapInfo);

	// 移動
	CheckResultMove(collisionMapInfo);

	CeilingCollisionMove(collisionMapInfo);

	isWallHit(collisionMapInfo);

	OnGroundSwitch(collisionMapInfo);

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = { std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> *3.0f / 2.0f };
		// 状態に応じた目標角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 自キャラの角度を設定する
		model_->SetRotate({ 0.0f, turnFirstRotationY_ + (EaseInOutSine(1.0f - (turnTimer_ / kTimeTurn)) * (destinationRotationY - turnFirstRotationY_)), 0.0f });
	}

	model_->Update();
}

void Player::Draw()
{
	model_->Draw();
}

void Player::Move()
{
	// 移動入力

	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (input_->PushKey(DIK_RIGHT) || input_->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (input_->PushKey(DIK_RIGHT)) {

				// 左入力中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = model_->GetTransform().rotate.y;
					turnTimer_ = kTimeTurn;
				}

			} else if (input_->PushKey(DIK_LEFT)) {

				// 右移動中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = model_->GetTransform().rotate.y;
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速 / 減速
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;
			velocity_.z += acceleration.z;

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (input_->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.x += Vector3(0, kJumpAcceleration, 0).x;
			velocity_.y += Vector3(0, kJumpAcceleration, 0).y;
			velocity_.z += Vector3(0, kJumpAcceleration, 0).z;
		}

	} else {
		// 落下速度
		velocity_.x += Vector3(0, -kGravityAcceleration, 0).x;
		velocity_.y += Vector3(0, -kGravityAcceleration, 0).y;
		velocity_.z += Vector3(0, -kGravityAcceleration, 0).z;
		// 落下速度制限
		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}

}

void Player::MapCollisionCheck(CollisionMapInfo& info)
{
	MapCollisionCheckUp(info);
	MapCollisionCheckDown(info);
	MapCollisionCheckRight(info);
	MapCollisionCheckLeft(info);
}

void Player::MapCollisionCheckUp(CollisionMapInfo& info)
{
	// 上昇あり？
	if (info.velocity.y <= 0) {
		return;
	}

	//移動後の4つの角の座標
	std::array<Vector3, 4> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(model_->GetTransform().translate + info.velocity, static_cast<Corner>(i));
	}

	// 移動前の4つの角の座標
	std::array<Vector3, 4> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		positions[i] = CornerPosition(model_->GetTransform().translate, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	//真上の当たり判定を行う
	bool hit = false;

	//左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	//右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	//ブロックにヒット？
	if (hit) {
		//めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
		//現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(positions[kLeftTop]);
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.y = (std::max)(0.0f, (rect.bottom - model_->GetTransform().translate.y) - ((kWidth / 2) + kBlank));
			// 天井に当たったことを記録する
			info.isCeilingCollision = true;
		}
	}
}

void Player::MapCollisionCheckDown(CollisionMapInfo& info)
{
	//下降あり？
	if (info.velocity.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, 4> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(model_->GetTransform().translate + info.velocity, static_cast<Corner>(i));
	}

	//移動前の4つの角の座標
	std::array<Vector3, 4> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		positions[i] = CornerPosition(model_->GetTransform().translate, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;

	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	//隣接セルがともにブロックであればヒットしない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒットしない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		//めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		//現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(positions[kLeftBottom]);
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.y = std::min(0.0f, (rect.top - model_->GetTransform().translate.y) + ((kHeight / 2) + kBlank));
			// 地面に当たったことを記録する
			info.isGroundCollision = true;
		}
	}
}

void Player::MapCollisionCheckRight(CollisionMapInfo& info)
{
	//右移動あり？
	if (info.velocity.x <= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, 4> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(model_->GetTransform().translate + info.velocity, static_cast<Corner>(i));
	}

	// 移動前の4つの角の座標
	std::array<Vector3, 4> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		positions[i] = CornerPosition(model_->GetTransform().translate, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 右の当たり判定を行う
	bool hit = false;

	// 右下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		//現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(positions[kRightBottom]);
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.x = (std::max)(0.0f, (rect.left - model_->GetTransform().translate.x) - ((kWidth / 2) + kBlank));
			// 壁に当たったことを記録する
			info.isWallCollision = true;
		}
	}
}

void Player::MapCollisionCheckLeft(CollisionMapInfo& info)
{
	// 左移動あり？
	if (info.velocity.x >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, 4> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(model_->GetTransform().translate + info.velocity, static_cast<Corner>(i));
	}

	// 移動前の4つの角の座標
	std::array<Vector3, 4> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		positions[i] = CornerPosition(model_->GetTransform().translate, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 左の当たり判定を行う
	bool hit = false;

	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		//現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(positions[kLeftBottom]);
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.x = std::min(0.0f, (rect.right - model_->GetTransform().translate.x) + ((kWidth / 2) + kBlank));
			// 壁に当たったことを記録する
			info.isWallCollision = true;
		}
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner)
{
	Vector3 offsetTable[kNumCorner] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0},	//kRightBottom
		{-kWidth / 2.0f, -kHeight / 2.0f, 0},	//kLeftBottom
		{+kWidth / 2.0f, +kHeight / 2.0f, 0},	//kRightTop
		{-kWidth / 2.0f, +kHeight / 2.0f, 0}	//kLeftTop
	};

	Vector3 a;
	a.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;
	a.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;
	a.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;

	return a;
}

void Player::CheckResultMove(const CollisionMapInfo& info)
{
	//移動
	model_->SetTranslate(model_->GetTransform().translate + info.velocity);
}

void Player::CeilingCollisionMove(const CollisionMapInfo& info)
{
	//天井に当たった？
	if (info.isCeilingCollision) {
		velocity_.y = 0;
	}
}

void Player::OnGroundSwitch(const CollisionMapInfo& info)
{
	//自キャラが接地状態？
	if (onGround_) {
		//ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {

			// 移動後の4つの角の座標
			std::array<Vector3, 4> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(model_->GetTransform().translate, static_cast<Corner>(i));
			}

			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;

			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kBlank, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右上点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kBlank, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			//落下開始
			if (!hit) {
				//空中状態に切り替える
				onGround_ = false;
			}

		}
	} else {
		if (info.isGroundCollision) {
			//着地状態に切り替える(落下を止める)
			onGround_ = true;
			//着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			//Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::isWallHit(const CollisionMapInfo& info)
{
	//壁接触による減速
	if (info.isWallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

float Player::EaseInOutSine(float t) {
	return -(std::cos(std::numbers::pi_v<float> * t) - 1) / 2;
}
