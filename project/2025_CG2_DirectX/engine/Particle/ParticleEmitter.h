#pragma once
#include "Transform.h"
#include <random>
#include <list>

struct Particle;

class ParticleManager;

enum class ParticleEffectType {
    Hit,
    Smoke,
};

class ParticleEmitter
{
public:
	
	void Initialize(std::list<Particle>* particlesPtr, ParticleManager* particleManager);

	void Update(std::mt19937& randomEngine);

	struct Transform GetTransform() const { return transform_; }

	uint32_t GetCount() const { return count_; }

private:

	struct Transform transform_;
	uint32_t count_;
	float frequency_;
	float frequencyTime_;

	std::list<Particle>* particlesPtr_ = nullptr;

	ParticleManager* particleManager_ = nullptr;

};

