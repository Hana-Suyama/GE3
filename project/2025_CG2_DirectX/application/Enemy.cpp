#include "Enemy.h"

#include "../engine/Time/TimeManager.h"
#include "../engine/Utility/Math/MyMath.h"
#include <algorithm>
#include <cmath>

using namespace MyMath;

void Enemy::Initialize(
	Object3DBasic* object3dBasic,
	ModelManager* modelManager,
	const Vector3& position,
	float patrolDistance,
	float moveSpeed)
{
	startX_ = position.x;
	patrolDistance_ = patrolDistance;
	moveSpeed_ = moveSpeed;
	direction_ = 1.0f;
	isDefeated_ = false;
	isDead_ = false;
	defeatTimer_ = 0.0f;

	model_ = std::make_unique<Object3D>();
	model_->Initialize(object3dBasic, modelManager, "resources/enemy.obj");
	model_->SetTranslate(position);
	ApplyFacingRotation();
	model_->Update();
}

void Enemy::Update()
{
	if (isDead_) {
		return;
	}

	if (isDefeated_) {
		UpdateDefeatAnimation();
		return;
	}

	Vector3 position = model_->GetTransform().translate;
	position.x += direction_ * moveSpeed_ * TimeManager::GetInstance()->GetDeltaTime();

	const float patrolLeft = startX_ - patrolDistance_;
	const float patrolRight = startX_ + patrolDistance_;

	if (position.x >= patrolRight) {
		position.x = patrolRight;
		direction_ = -1.0f;
		ApplyFacingRotation();
	} else if (position.x <= patrolLeft) {
		position.x = patrolLeft;
		direction_ = 1.0f;
		ApplyFacingRotation();
	}

	model_->SetTranslate(position);
	model_->Update();
}

void Enemy::Draw()
{
	if (isDead_) {
		return;
	}

	model_->Draw();
}

bool Enemy::IsCollidingWithPlayer(const Vector3& playerPosition) const
{
	if (isDead_ || isDefeated_) {
		return false;
	}

	const Vector3& enemyPosition = GetPosition();

	return std::abs(playerPosition.x - enemyPosition.x) <= kCombinedCollisionHalfWidth &&
		std::abs(playerPosition.y - enemyPosition.y) <= kCombinedCollisionHalfHeight;
}

bool Enemy::IsStompedByPlayer(const Vector3& playerPosition, const Vector3& playerVelocity) const
{
	if (isDead_ || isDefeated_ || playerVelocity.y >= 0.0f) {
		return false;
	}

	const Vector3& enemyPosition = GetPosition();
	const float playerBottom = playerPosition.y - kPlayerHalfHeight;
	const float previousPlayerBottom = playerBottom - playerVelocity.y;
	const float enemyTop = enemyPosition.y + kEnemyHalfHeight;

	const bool isHorizontallyOverlapping =
		std::abs(playerPosition.x - enemyPosition.x) <= kCombinedCollisionHalfWidth;
	const bool wasAboveEnemy = previousPlayerBottom >= enemyTop - kStompTolerance;
	const bool reachedEnemyTop = playerBottom <= enemyTop + kStompPenetrationAllowance;

	return isHorizontallyOverlapping && wasAboveEnemy && reachedEnemyTop;
}

void Enemy::Defeat()
{
	if (isDead_ || isDefeated_) {
		return;
	}

	isDefeated_ = true;
	defeatTimer_ = kDefeatAnimationTime * kDefeatStartProgress;
	defeatStartPosition_ = GetPosition();
	ApplyDefeatPose(kDefeatStartProgress);
}

const Vector3& Enemy::GetPosition() const
{
	return model_->GetTransform().translate;
}

void Enemy::ApplyFacingRotation()
{
	const float rotateY = direction_ > 0.0f ? DEGtoRAD(90.0f) : DEGtoRAD(270.0f);
	model_->SetRotate({ 0.0f, rotateY, 0.0f });
}

void Enemy::UpdateDefeatAnimation()
{
	defeatTimer_ += TimeManager::GetInstance()->GetDeltaTime();

	const float progress = std::clamp(defeatTimer_ / kDefeatAnimationTime, 0.0f, 1.0f);
	ApplyDefeatPose(progress);

	if (defeatTimer_ >= kDefeatAnimationTime + kDefeatHoldTime) {
		isDead_ = true;
	}
}

void Enemy::ApplyDefeatPose(float progress)
{
	const float inverseProgress = 1.0f - progress;
	const float easedProgress = 1.0f - inverseProgress * inverseProgress * inverseProgress;

	const Vector3 scale = {
		1.0f + (kDefeatScaleX - 1.0f) * easedProgress,
		1.0f + (kDefeatScaleY - 1.0f) * easedProgress,
		1.0f + (kDefeatScaleZ - 1.0f) * easedProgress
	};

	Vector3 position = defeatStartPosition_;
	position.y -= kEnemyHalfHeight * (1.0f - scale.y);

	model_->SetScale(scale);
	model_->SetTranslate(position);
	model_->Update();
}
