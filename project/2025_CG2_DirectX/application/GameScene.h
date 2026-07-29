#pragma once
#include "Coin.h"
#include "Enemy.h"
#include "Player.h"
#include "CameraController.h"
#include "../engine/Scene/BaseScene.h"
#include "../engine/Sprite/Sprite.h"
#include <LevelLoader/LevelLoader.h>
#include <array>

class GameScene : public BaseScene
{
public:

	virtual void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, SkinnedObject3DBasic* skinnedObject3DBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine);

	virtual void Update();

	virtual void SpriteDraw();

	virtual void ModelDraw();

	virtual void SkinnedModelDraw();

	virtual void ImGuiDraw();

	virtual void Finalize();

	void GenerateBlocks();

	bool GetIsClear() { return isClear_; };

	void GenerateLevelObjects();

private:

	void RespawnPlayer();
	void InitializeTimerDisplay();
	void UpdateTimerDisplay();

	DirectXBasic* directXBasic_ = nullptr;
	Object3DBasic* object3dBasic_ = nullptr;
	SkinnedObject3DBasic* skinnedObject3dBasic_ = nullptr;
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
	struct EulerTransform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -30.0f } };

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
	Vector3 playerSpawnPosition_{};

	std::vector<std::unique_ptr<Enemy>> enemies_;
	std::vector<std::unique_ptr<Coin>> coins_;
	uint32_t collectedCoinCount_ = 0;
	uint32_t totalCoinCount_ = 0;

	static constexpr size_t kTimerDigitCount = 6;
	std::array<std::unique_ptr<Sprite>, kTimerDigitCount> timerDigitSprites_;
	std::array<std::string, 10> timerDigitTexturePaths_;
	float elapsedTime_ = 0.0f;

	std::unique_ptr<Vector3> planePosition{};

	std::unique_ptr<LevelData> levelData_;
	std::vector<std::unique_ptr<Object3D>> levelObjects_;

};

