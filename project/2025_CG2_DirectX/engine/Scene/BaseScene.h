#pragma once
#include "Object3DBasic.h"
#include "ModelManager.h"
#include "Input.h"
#include "Object3D.h"
#include "ParticleManager.h"
#include "Sprite.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "CameraForGPU.h"
#include "ImGuiManager.h"
#include "XAudio2Basic.h"

class SceneManager;

class BaseScene
{
public:

	virtual ~BaseScene() = default;

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine) = 0;

	virtual void Update() = 0;

	virtual void SpriteDraw() = 0;

	virtual void ModelDraw() = 0;

	virtual void ImGuiDraw() = 0;

	virtual void Finalize() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

protected:

	SceneManager* sceneManager_ = nullptr;

};

