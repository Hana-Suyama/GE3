#pragma once

#include "../engine/Model/ModelManager.h"
#include "../engine/Object3D/Object3D.h"
#include "../engine/Object3D/Object3DBasic.h"
#include "../engine/Utility/Math/Vector3.h"
#include <memory>

class Coin
{
public:
	void Initialize(
		Object3DBasic* object3dBasic,
		ModelManager* modelManager,
		const Vector3& position);

	void Update();

	void Draw();

	bool TryCollect(const Vector3& playerPosition);

private:
	void UpdateCollectionAnimation(float deltaTime);

	float EaseInQuad(float t) const;

	float EaseOutCubic(float t) const;

	std::unique_ptr<Object3D> model_ = nullptr;
	Vector3 basePosition_{};
	Vector3 collectionStartPosition_{};
	float animationTimer_ = 0.0f;
	float collectionTimer_ = 0.0f;
	bool isCollecting_ = false;
	bool isCollected_ = false;

	static inline const float kRotationSpeed = 3.5f;
	static inline const float kBobSpeed = 4.0f;
	static inline const float kBobHeight = 0.12f;
	static inline const float kCollisionHalfWidth = 0.65f;
	static inline const float kCollisionHalfHeight = 0.75f;
	static inline const float kCollectionAnimationTime = 0.5f;
	static inline const float kCollectionRiseHeight = 1.2f;
	static inline const float kCollectionMaxRotationSpeed = 55.0f;
};
