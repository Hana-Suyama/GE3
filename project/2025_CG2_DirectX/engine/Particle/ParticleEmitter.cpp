#include "ParticleEmitter.h"
#include <TimeManager.h>
#include "ImGuiManager.h"
#include "ParticleManager.h"

void ParticleEmitter::Initialize(std::list<Particle>* particlesPtr, ParticleManager* particleManager)
{
	count_ = 8;
	frequency_ = 2.0f;
	frequencyTime_ = 0.0f;

	transform_.translate = { 0.0f, 3.0f, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };

	particlesPtr_ = particlesPtr;
	particleManager_ = particleManager;
}

void ParticleEmitter::Update(std::mt19937& randomEngine)
{
	frequencyTime_ += TimeManager::GetInstance()->GetDeltaTime();
	if (frequency_ <= frequencyTime_) {
		particlesPtr_->splice(particlesPtr_->end(), particleManager_->Emit(*this, randomEngine));
		frequencyTime_ -= frequency_;
	}

#ifdef USE_IMGUI
	ImGui::Begin("Particle");
	
	ImGui::DragFloat3("EmitterTranslate", &transform_.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();

#endif
}
