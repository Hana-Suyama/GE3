#pragma once
#include "../../engine/Scene/BaseScene.h"
#include <Skybox/SkyBoxBasic.h>
#include <Skybox/SkyBox.h>
#include "../../engine/LevelLoader/LevelLoader.h"

class SampleScene : public BaseScene
{
public:

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine);

	virtual void Update();

	virtual void SpriteDraw();

	virtual void ModelDraw();

	virtual void ImGuiDraw();

	virtual void Finalize();

	void SetLight();

	void GenerateLevelObjects();


private:

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Logger* logger_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteBasic* spriteBasic_ = nullptr;
	std::mt19937* randomEngine_ = nullptr;

	std::unique_ptr<ParticleManager> particleManager = nullptr;

	std::unique_ptr<Sprite> sprite = nullptr;
	std::unique_ptr<Sprite> sprite2 = nullptr;

	std::unique_ptr<Object3D> object3d = nullptr;
	std::unique_ptr<Object3D> object3dTeapot= nullptr;
	std::unique_ptr<Object3D> object3dMultiMesh = nullptr;
	std::unique_ptr<Object3D> object3dMultiMaterial = nullptr;
	std::unique_ptr<Object3D> object3dBunny = nullptr;
	std::unique_ptr<Object3D> object3dSuzanne = nullptr;
	std::unique_ptr<Object3D> object3dTerrain = nullptr;
	std::unique_ptr<Object3D> object3dPlanegLTF = nullptr;
	std::unique_ptr<Object3D> object3dSphere = nullptr;
	std::unique_ptr<Object3D> object3dAnimCube = nullptr;
	std::unique_ptr<Object3D> object3dPlayer = nullptr;

	std::unique_ptr<SkyBoxBasic> skyBoxBasic_ = nullptr;
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	std::unique_ptr<Camera> camera = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource;

	CameraForGPU* cameraForGPUData = nullptr;

	std::vector<std::unique_ptr<Light>> lights_;

	// ライトを一括でGPUに送るためのバッファとデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> lightsBufferResource_;
	LightBuffer* lightsBufferData_ = nullptr;


	XAudio2Basic* xaudio2Basic_ = nullptr;

	//カメラ用変数を作る
	struct Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -30.0f } };

	bool playSound = false;

	Vector3 EmitterPosition{};

	std::unique_ptr<LevelData> levelData_;
	std::vector<std::unique_ptr<Object3D>> levelObjects_;
	Animation animation;
	std::vector<std::unique_ptr<Object3D>> enemyObjects_;

};

