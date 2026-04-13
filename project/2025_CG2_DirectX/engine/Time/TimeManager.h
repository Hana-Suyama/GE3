#pragma once
#include <windows.h>
#include <memory>

class TimeManager
{
private:

	/* --------- シングルトン化 --------- */

	static std::unique_ptr<TimeManager> instance_;

	TimeManager(const TimeManager&) = delete;
	TimeManager& operator=(const TimeManager&) = delete;
public:
	TimeManager() = default;
	~TimeManager() = default;

public:

	/* --------- public関数 --------- */

	static TimeManager* GetInstance();
	static void ReleaseInstance();

	void Initialize();
	void Update();

	float GetDeltaTime() const;
	float GetUnscaledDeltaTime() const;

	void SetTimeScale(float scale);
	float GetTimeScale() const;

	void Pause();
	void Resume();

private:
	LARGE_INTEGER freq_{};
	LARGE_INTEGER prev_{};
	LARGE_INTEGER curr_{};

	float deltaTime_ = 0.0f;
	float timeScale_ = 1.0f;
	bool paused_ = false;
};