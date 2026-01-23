#pragma once
#include "BaseScene.h"

class SceneManager
{
public:

	~SceneManager();

	void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine);

	void Update();

	void SpriteDraw();

	void ModelDraw();

	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = move(nextScene); }

private:

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Logger* logger_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteBasic* spriteBasic_ = nullptr;
	std::mt19937* randomEngine_ = nullptr;
	XAudio2Basic* xaudio2Basic_ = nullptr;

	std::unique_ptr<BaseScene> scene_ = nullptr;

	std::unique_ptr<BaseScene> nextScene_ = nullptr;

};

