#pragma once
#include "../engine/Object3D/Object3D.h"
#include "Player.h"

class GameScene
{
public:

	void Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Input* input, Camera* camera);

	void Update();

	void Draw();

	void Finalize();

private:

	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Input* input_ = nullptr;
	Camera* camera_ = nullptr;

	std::vector<Object3D*> blocks_;

	Object3D* playerModel_ = nullptr;

	Player* player_ = nullptr;

	Vector3 planePosition{};

};

