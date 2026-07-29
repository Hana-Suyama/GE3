#include "Coin.h"

#include "../engine/Material.h"
#include "../engine/Time/TimeManager.h"
#include <algorithm>
#include <cmath>

void Coin::Initialize(
	Object3DBasic* object3dBasic,
	ModelManager* modelManager,
	const Vector3& position)
{
	basePosition_ = position;
	animationTimer_ = 0.0f;
	collectionTimer_ = 0.0f;
	isCollecting_ = false;
	isCollected_ = false;

	model_ = std::make_unique<Object3D>();
	model_->Initialize(object3dBasic, modelManager, "resources/coin.obj");
	model_->SetTranslate(basePosition_);

	for (Material* material : model_->GetMaterialData()) {
		material->color = { 1.0f, 0.72f, 0.05f, 1.0f };
		material->enableLighting = Reflectance::HalfLambert;
		material->enableReflection = Reflection::BlinnPhongReflection;
		material->shininess = 6.0f;
	}

	model_->Update();
}

void Coin::Update()
{
	if (isCollected_) {
		return;
	}

	const float deltaTime = TimeManager::GetInstance()->GetDeltaTime();

	if (isCollecting_) {
		UpdateCollectionAnimation(deltaTime);
		return;
	}

	animationTimer_ += deltaTime;

	Vector3 position = basePosition_;
	position.y += std::sin(animationTimer_ * kBobSpeed) * kBobHeight;

	Vector3 rotation = model_->GetTransform().rotate;
	rotation.y += kRotationSpeed * deltaTime;

	model_->SetTranslate(position);
	model_->SetRotate(rotation);
	model_->Update();
}

void Coin::Draw()
{
	if (!isCollected_) {
		model_->Draw();
	}
}

bool Coin::TryCollect(const Vector3& playerPosition)
{
	if (isCollected_ || isCollecting_) {
		return false;
	}

	const Vector3& coinPosition = model_->GetTransform().translate;
	const bool isHit =
		std::abs(playerPosition.x - coinPosition.x) <= kCollisionHalfWidth &&
		std::abs(playerPosition.y - coinPosition.y) <= kCollisionHalfHeight;

	if (isHit) {
		isCollecting_ = true;
		collectionTimer_ = 0.0f;
		collectionStartPosition_ = coinPosition;
		return true;
	}

	return false;
}

void Coin::UpdateCollectionAnimation(float deltaTime)
{
	collectionTimer_ += deltaTime;
	const float progress = (std::min)(
		collectionTimer_ / kCollectionAnimationTime,
		1.0f);

	Vector3 position = collectionStartPosition_;
	position.y += kCollectionRiseHeight * EaseOutCubic(progress);

	Vector3 rotation = model_->GetTransform().rotate;
	const float rotationSpeed =
		kRotationSpeed +
		(kCollectionMaxRotationSpeed - kRotationSpeed) * EaseInQuad(progress);
	rotation.y += rotationSpeed * deltaTime;

	model_->SetTranslate(position);
	model_->SetRotate(rotation);
	model_->Update();

	if (progress >= 1.0f) {
		isCollected_ = true;
		isCollecting_ = false;
	}
}

float Coin::EaseInQuad(float t) const
{
	return t * t;
}

float Coin::EaseOutCubic(float t) const
{
	const float inverse = 1.0f - t;
	return 1.0f - inverse * inverse * inverse;
}
