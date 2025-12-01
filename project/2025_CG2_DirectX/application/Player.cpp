#include "Player.h"
#include <algorithm>
#include"../engine/utility/Math/MyMath.h"
#include <numbers>

void Player::Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Object3D* model, Input* input, Camera* camera)
{
	object3dBasic_ = object3dBasic;
	modelManager_ = modelManager;
	model_ = model;
	input_ = input;
	camera_ = camera;
}

void Player::Update()
{

	// 左右移動操作
	if ((input_->PushKey(DIK_RIGHT)) || (input_->PushKey(DIK_LEFT))) {

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
	Vector3 translate = model_->GetTransform().translate;
	model_->SetTranslate(MyMath::Add(translate, velocity_));

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = { std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> *3.0f / 2.0f };
		// 状態に応じた目標角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 自キャラの角度を設定する
		model_->SetRotate({0.0f, turnFirstRotationY_ + (EaseInOutSine(1.0f - (turnTimer_ / kTimeTurn)) * (destinationRotationY - turnFirstRotationY_)), 0.0f, });
	}

	model_->Update();

	//追従対象のワールドトランスフォームを参照
	const Vector3& targetWorldTransform = model_->GetTransform().translate;
	const Vector3& targetVelocity = velocity_;
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetPositon_.x = targetWorldTransform.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
	targetPositon_.y = targetWorldTransform.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
	targetPositon_.z = targetWorldTransform.z + targetOffset_.z + targetVelocity.z * kVelocityBias;

	//座標補間によりゆったり追従
	/*float x = KamataEngine::MathUtility::Lerp(camera_.translation_.x, targetPositon_.x, kInterpolationRate);
	float y = KamataEngine::MathUtility::Lerp(camera_.translation_.y, targetPositon_.y, kInterpolationRate);
	float z = KamataEngine::MathUtility::Lerp(camera_.translation_.z, targetPositon_.z, kInterpolationRate);*/

	////追従対象が画面外に出ないように補正
	//camera_.translation_.x = std::max(camera_.translation_.x, targetWorldTransform.translation_.x + kMargin.left);
	//camera_.translation_.x = std::min(camera_.translation_.x, targetWorldTransform.translation_.x + kMargin.right);
	//camera_.translation_.y = std::max(camera_.translation_.y, targetWorldTransform.translation_.y + kMargin.bottom);
	//camera_.translation_.y = std::min(camera_.translation_.y, targetWorldTransform.translation_.y + kMargin.top);

	////移動範囲制限
	//camera_.translation_.x = std::max(camera_.translation_.x, movableArea_.left);
	//camera_.translation_.x = std::min(camera_.translation_.x, movableArea_.right);
	//camera_.translation_.y = std::max(camera_.translation_.y, movableArea_.bottom);
	//camera_.translation_.y = std::min(camera_.translation_.y, movableArea_.top);

	camera_->SetTranslate(targetPositon_);
}

void Player::Draw()
{
	model_->Draw();
}

float Player::EaseInOutSine(float t) {
	return -(std::cos(std::numbers::pi_v<float> * t) - 1) / 2;
}
