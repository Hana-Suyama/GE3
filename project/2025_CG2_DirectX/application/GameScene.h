#pragma once
#include "Player.h"
#include "CameraController.h"
#include "../engine/Scene/BaseScene.h"

class GameScene : public BaseScene
{
public:

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine);

	virtual void Update();

	virtual void SpriteDraw();

	virtual void ModelDraw();

	virtual void ImGuiDraw();

	virtual void Finalize();

	void GenerateBlocks();

	bool GetIsClear() { return isClear_; };

private:

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	Logger* logger_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	SpriteBasic* spriteBasic_ = nullptr;
	std::mt19937* randomEngine_ = nullptr;


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

	//カメラコントローラ
	std::unique_ptr<CameraController> cameraController_ = nullptr;


	std::vector<std::unique_ptr<Object3D>> blocks_;
	//ブロック用のワールドトランスフォーム
	std::vector<std::vector<std::unique_ptr<Vector3>>> worldTransformBlocks_;

	//マップチップフィールド
	std::unique_ptr<MapChipField> mapChipField_;

	bool isClear_ = false;


	std::unique_ptr<Object3D> playerModel_ = nullptr;

	std::unique_ptr<Object3D> flag_ = nullptr;

	std::unique_ptr<Player> player_ = nullptr;

	std::unique_ptr<Vector3> planePosition{};

};

