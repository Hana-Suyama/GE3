#pragma once
#include "../../engine/Object3D/Object3DBasic.h"
#include "../../engine/Model/ModelManager.h"
#include "../../engine/Input/Input.h"
#include "../../engine/Object3D/Object3D.h"
#include "../../engine/Particle/ParticleManager.h"
#include "../../engine/Sprite/Sprite.h"
#include "../../engine/Light/DirectionalLight.h"
#include "../../engine/Light/PointLight.h"
#include "../../engine/Light/SpotLight.h"
#include "../../engine/Camera/CameraForGPU.h"
#include "../../engine/debug/ImGui/ImGuiManager.h"
#include "../../engine/Audio/XAudio2Basic.h"

class SceneManager;

class BaseScene
{
public:

	virtual ~BaseScene() = default;

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine) = 0;

	virtual void Update() = 0;

	virtual void SpriteDraw() = 0;

	virtual void ModelDraw() = 0;

	virtual void Finalize() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

protected:

	SceneManager* sceneManager_ = nullptr;

};

