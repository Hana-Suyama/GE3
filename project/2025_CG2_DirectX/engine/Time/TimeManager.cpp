#include "TimeManager.h"
#include <algorithm>

std::unique_ptr<TimeManager> TimeManager::instance_;

TimeManager* TimeManager::GetInstance()
{
	if (!instance_)
	{
		instance_ = std::make_unique<TimeManager>();
		instance_->Initialize();
	}
	return instance_.get();
}

void TimeManager::ReleaseInstance()
{
	instance_.reset();
}

void TimeManager::Initialize()
{
	QueryPerformanceFrequency(&freq_);
	QueryPerformanceCounter(&prev_);
}

void TimeManager::Update()
{
	QueryPerformanceCounter(&curr_);

	LONGLONG diff = curr_.QuadPart - prev_.QuadPart;
	float sec = static_cast<float>(diff) / static_cast<float>(freq_.QuadPart);

	prev_ = curr_;

	deltaTime_ = paused_ ? 0.0f : sec * timeScale_;

	deltaTime_ = std::clamp(deltaTime_, 0.0f, 0.1f);
}

float TimeManager::GetDeltaTime() const
{
	return deltaTime_;
}

float TimeManager::GetUnscaledDeltaTime() const
{
	return deltaTime_ / timeScale_;
}

void TimeManager::SetTimeScale(float scale)
{
	timeScale_ = scale;
}

float TimeManager::GetTimeScale() const
{
	return timeScale_;
}

void TimeManager::Pause()
{
	paused_ = true;
}

void TimeManager::Resume()
{
	paused_ = false;
}