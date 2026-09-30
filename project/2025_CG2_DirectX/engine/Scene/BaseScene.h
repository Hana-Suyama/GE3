#pragma once

#include <random>

class SceneManager;
class PostEffectController;
class DirectXBasic;
class Object3DBasic;
class SkinnedObject3DBasic;
class ModelManager;
class Logger;
class SRVManager;
class TextureManager;
class SpriteBasic;
class XAudio2Basic;

class BaseScene
{
public:

	virtual ~BaseScene() = default;

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, SkinnedObject3DBasic* skinnedObject3DBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine) = 0;

	virtual void Update() = 0;

	virtual void SpriteDraw() = 0;

	virtual void ModelDraw() = 0;

	virtual void SkinnedModelDraw() = 0;

	virtual void ImGuiDraw() = 0;

	virtual void Finalize() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
	virtual void SetPostEffectController(PostEffectController* postEffectController) { postEffectController_ = postEffectController; }

protected:

	SceneManager* sceneManager_ = nullptr;
	PostEffectController* postEffectController_ = nullptr;

};

