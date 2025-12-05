#pragma once
#include "../engine/Engine.h"

class Game : public Engine
{
	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

private:

	GameScene* gameScene = nullptr;
	SampleScene* sampleScene = nullptr;

};

