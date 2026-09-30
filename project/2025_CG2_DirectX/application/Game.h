#pragma once
#include "Engine.h"

class Game : public Engine
{

	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

};

