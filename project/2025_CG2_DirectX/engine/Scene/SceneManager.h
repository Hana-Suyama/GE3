#pragma once

#include <memory>
#include <random>

class BaseScene;
class DirectXBasic;
class Object3DBasic;
class SkinnedObject3DBasic;
class ModelManager;
class Logger;
class SRVManager;
class TextureManager;
class SpriteBasic;
class XAudio2Basic;
class PostEffectController;

class SceneManager
{
public:

	SceneManager();
	~SceneManager();

	void Initialize(
		DirectXBasic* directXBasic,
		Object3DBasic* object3dBasic,
		SkinnedObject3DBasic* skinnedObject3dBasic,
		ModelManager* modelManager,
		Logger* logger,
		SRVManager* srvManager,
		TextureManager* textureManager,
		SpriteBasic* spriteBasic,
		XAudio2Basic* xaudio2Basic,
		std::mt19937* randomEngine,
		PostEffectController* postEffectController);

	void Update();

	void SpriteDraw();

	void ModelDraw();

	void SkinnedModelDraw();

	void ImGuiDraw();

	void RequestSceneChange(std::unique_ptr<BaseScene> nextScene);

	void ActivatePendingScene();

	void FinalizeCurrentScene();

private:

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	SkinnedObject3DBasic* skinnedObject3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Logger* logger_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteBasic* spriteBasic_ = nullptr;
	std::mt19937* randomEngine_ = nullptr;
	XAudio2Basic* xaudio2Basic_ = nullptr;
	PostEffectController* postEffectController_ = nullptr;

	std::unique_ptr<BaseScene> scene_ = nullptr;
	std::unique_ptr<BaseScene> pendingScene_ = nullptr;

};

