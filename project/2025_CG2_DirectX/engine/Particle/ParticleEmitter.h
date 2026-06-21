#pragma once
#include "Transform.h"
#include <random>
#include <list>
#include "Vector4.h"

struct Particle;

class ParticleManager;

enum class ParticleEffectType {
    Hit,
    Smoke,
	Circle,
	Cylinder,
};

struct ParticleSpawnSettings {
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Vector3 rotate = { 0.0f, 0.0f, 0.0f };
	Vector3 velocityMin = { 0.0f, 0.0f, 0.0f };
	Vector3 velocityMax = { 0.0f, 0.0f, 0.0f };
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float lifeTime = 1.0f;
};

class ParticleEmitter
{
public:
	
	void Initialize(std::list<Particle>* particlesPtr, ParticleManager* particleManager, ParticleEffectType effectType);

	void Update(std::mt19937& randomEngine);

	const struct Transform& GetTransform() const { return transform_; }

	uint32_t GetCount() const { return count_; }

	ParticleEffectType GetEffectType() const { return effectType_; }

private:

	struct Transform transform_;
	uint32_t count_;
	float frequency_;
	float frequencyTime_;
	ParticleEffectType effectType_ = ParticleEffectType::Hit;

	std::list<Particle>* particlesPtr_ = nullptr;

	ParticleManager* particleManager_ = nullptr;

};

