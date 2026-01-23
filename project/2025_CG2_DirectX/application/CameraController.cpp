#define NOMINMAX
#include "CameraController.h"
#include <algorithm>
#include "../engine/Utility/Math/Lerp.h"

void CameraController::Initialize(Camera* camera) {
	camera_ = camera;
}

void CameraController::Update() {
	//追従対象のワールドトランスフォームを参照
	const Vector3 targetWorldTransform = target_->GetWorldTransform();
	const Vector3& targetVelocity = target_->GetVelocity();
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetPosition_ = targetWorldTransform + targetOffset_ + targetVelocity * kVelocityBias;

	Vector3 result{};
	//座標補間によりゆったり追従
	result = MyMath::Lerp(camera_->GetTranslate(), targetPosition_, kInterpolationRate);
	
	//追従対象が画面外に出ないように補正
	result.x = std::max(result.x, targetWorldTransform.x + kMargin.left);
	result.x = std::min(result.x, targetWorldTransform.x + kMargin.right);
	result.y = std::max(result.y, targetWorldTransform.y + kMargin.bottom);
	result.y = std::min(result.y, targetWorldTransform.y + kMargin.top);

	//移動範囲制限
	result.x = (std::max)(result.x, movableArea_.left);
	result.x = (std::min)(result.x, movableArea_.right);
	result.y = std::max(result.y, movableArea_.bottom);
	result.y = std::min(result.y, movableArea_.top);


	camera_->SetTranslate(result);
	//行列を更新する
	//camera_.UpdateMatrix();
}

void CameraController::Reset() {
	//追従対象のワールドトランスフォームを参照
	const Vector3& targetWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの座標を計算
	Vector3 result{};
	result.x = targetWorldTransform.x + targetOffset_.x;
	result.y = targetWorldTransform.y + targetOffset_.y;
	result.z = targetWorldTransform.z + targetOffset_.z;
	camera_->SetTranslate(result);
}