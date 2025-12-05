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

class SampleScene
{
public:

	void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Input* input, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine);

	void Update();

	void SpriteDraw();

	void ModelDraw();

	void Finalize();

	void SetLight();

private:

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Input* input_ = nullptr;
	Logger* logger_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteBasic* spriteBasic_ = nullptr;
	std::mt19937* randomEngine_ = nullptr;

	ParticleManager* particleManager = nullptr;

	Sprite* sprite = nullptr;
	Sprite* sprite2 = nullptr;

	Object3D* object3d = nullptr;
	Object3D* object3dTeapot = nullptr;
	Object3D* object3dMultiMesh = nullptr;
	Object3D* object3dMultiMaterial = nullptr;
	Object3D* object3dBunny = nullptr;
	Object3D* object3dSuzanne = nullptr;
	Object3D* object3dTerrain = nullptr;
	Object3D* object3dPlanegLTF = nullptr;

	Camera* camera = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;

	DirectionalLight* directionalLightData = nullptr;
	CameraForGPU* cameraForGPUData = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;

	XAudio2Basic* xaudio2Basic_ = nullptr;

	//カメラ用変数を作る
	struct Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -30.0f } };

	bool playSound = false;

	Vector3 EmitterPosition{};



};

