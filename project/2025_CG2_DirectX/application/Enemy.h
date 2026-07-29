#pragma once

#include "../engine/Model/ModelManager.h"
#include "../engine/Object3D/Object3D.h"
#include "../engine/Object3D/Object3DBasic.h"
#include "../engine/Utility/Math/Vector3.h"
#include <memory>

class Enemy
{
public:
	void Initialize(
		Object3DBasic* object3dBasic,
		ModelManager* modelManager,
		const Vector3& position,
		float patrolDistance = 3.0f,
		float moveSpeed = 2.0f);

	void Update();

	void Draw();

	bool IsCollidingWithPlayer(const Vector3& playerPosition) const;

	bool IsStompedByPlayer(const Vector3& playerPosition, const Vector3& playerVelocity) const;

	void Defeat();

	const Vector3& GetPosition() const;

private:
	void ApplyFacingRotation();

	void UpdateDefeatAnimation();

	void ApplyDefeatPose(float progress);

	std::unique_ptr<Object3D> model_ = nullptr;

	float startX_ = 0.0f;
	float patrolDistance_ = 3.0f;
	float moveSpeed_ = 2.0f;
	float direction_ = 1.0f;
	bool isDefeated_ = false;
	bool isDead_ = false;
	float defeatTimer_ = 0.0f;
	Vector3 defeatStartPosition_{};

	static inline const float kCombinedCollisionHalfWidth = 0.85f;
	static inline const float kCombinedCollisionHalfHeight = 0.85f;
	static inline const float kPlayerHalfHeight = 0.4f;
	static inline const float kEnemyHalfHeight = 0.45f;
	static inline const float kStompTolerance = 0.1f;
	static inline const float kStompPenetrationAllowance = 0.3f;
	static inline const float kDefeatAnimationTime = 0.18f;
	static inline const float kDefeatHoldTime = 0.15f;
	static inline const float kDefeatStartProgress = 0.12f;
	static inline const float kDefeatScaleY = 0.1f;
	static inline const float kDefeatScaleX = 1.35f;
	static inline const float kDefeatScaleZ = 1.15f;
};
