#pragma once
#include "../engine/Engine.h"
#include "TitleScene.h"
#include "ClearScene.h"

class Game : public Engine
{

	enum class Scene {
		GameScene,
		TitleScene,
		ClearScene,
		SampleScene,
	};

	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

private:

	std::unique_ptr<GameScene> gameScene = nullptr;
	std::unique_ptr<SampleScene> sampleScene = nullptr;
	std::unique_ptr<TitleScene> titleScene = nullptr;
	std::unique_ptr<ClearScene> clearScene = nullptr;

	Scene currentScene_;

};

