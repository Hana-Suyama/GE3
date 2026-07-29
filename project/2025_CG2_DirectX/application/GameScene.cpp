#include "GameScene.h"
#include "../engine/Light/DirectionalLight.h"
#include "../engine/Camera/CameraForGPU.h"
#include "../engine/Light/PointLight.h"
#include "../engine/Light/SpotLight.h"
#include <numbers>
#include "ClearScene.h"
#include "../engine/Scene/SceneManager.h"
#include "../engine/Time/TimeManager.h"

using namespace MyMath;

void GameScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, SkinnedObject3DBasic* skinnedObject3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
{
	directXBasic_ = directXBasic;
	object3dBasic_ = object3dBasic;
	skinnedObject3dBasic_ = skinnedObject3dBasic;
	modelManager_ = modelManager;
	logger_ = logger;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
	spriteBasic_ = spriteBasic;
	xaudio2Basic_ = xaudio2Basic;
	randomEngine_ = randomEngine;

	modelManager->LoadModel("resources", "player.obj");
	modelManager->LoadModel("resources", "Block.obj");
	modelManager->LoadModel("resources", "flag.obj");
	modelManager->LoadModelAssimp("resources", "enemy.obj");
	modelManager->LoadModel("resources", "coin.obj");

	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");
	InitializeTimerDisplay();

	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	object3dBasic_->SetDefaultCamera(camera.get());

	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	flag_ = std::make_unique<Object3D>();
	flag_->Initialize(object3dBasic_, modelManager_, "resources/flag.obj");
	flag_->SetTranslate(mapChipField_->GetMapChipPositionByIndex(196, 18));
	flag_->SetRotate({ 0.0f, DEGtoRAD(90.0f), 0.0f });

	GenerateBlocks();

	// カメラ位置転送用のリソースを作る
	cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	playerModel_ = std::make_unique<Object3D>();
	playerModel_->Initialize(object3dBasic_, modelManager_, "resources/player.obj");
	playerSpawnPosition_ = mapChipField_->GetMapChipPositionByIndex(3, 18);
	playerModel_->SetTranslate(playerSpawnPosition_);

	GenerateLevelObjects();

	player_ = std::make_unique<Player>();
	player_->Initialize(object3dBasic_, modelManager_, playerModel_.get(), camera.get());
	player_->SetMapChipField(mapChipField_.get());

	//カメラコントローラの生成
	cameraController_ = std::make_unique<CameraController>();
	//カメラコントローラの初期化
	cameraController_->Initialize(camera.get());
	//追従対象をセット
	cameraController_->SetTarget(player_.get());
	cameraController_->Reset();

	lights_.push_back(std::make_unique<DirectionalLight>());
	lights_.back()->Initialize();

	lights_.push_back(std::make_unique<PointLight>());
	lights_.back()->Initialize();

	lights_.push_back(std::make_unique<SpotLight>());
	lights_.back()->Initialize();

	// ライトを一括でGPUに送るためのバッファを作る
	lightsBufferResource_ = directXBasic->CreateBufferResource(sizeof(LightBuffer));
	// データを書き込む
	lightsBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightsBufferData_));
}

void GameScene::Update()
{
	if (!isClear_) {
		elapsedTime_ += TimeManager::GetInstance()->GetDeltaTime();
		UpdateTimerDisplay();
	}

	if (isClear_) {
		std::unique_ptr<BaseScene> scene =
			std::make_unique<ClearScene>(collectedCoinCount_, totalCoinCount_);
		sceneManager_->SetNextScene(move(scene));
	}

	if(playerModel_->GetTransform().translate.x >= 196.0f) {
		isClear_ = true;
	}

	cameraController_->Update();
	camera->Update();

	for (auto& block : blocks_) {
		block->Update();
	}

	player_->Update();

	for (auto& enemy : enemies_) {
		enemy->Update();
	}

	for (auto& coin : coins_) {
		coin->Update();
		if (coin->TryCollect(player_->GetWorldTransform())) {
			++collectedCoinCount_;
		}
	}

	flag_->Update();

	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	bool shouldRespawn = player_->GetWorldTransform().y <= -5.0f;
	if (!shouldRespawn) {
		for (auto& enemy : enemies_) {
			if (!enemy->IsCollidingWithPlayer(player_->GetWorldTransform())) {
				continue;
			}

			if (enemy->IsStompedByPlayer(player_->GetWorldTransform(), player_->GetVelocity())) {
				enemy->Defeat();
				player_->BounceFromEnemy();
				break;
			}

			shouldRespawn = true;
			break;
		}
	}

	if (shouldRespawn) {
		RespawnPlayer();
	}

}

void GameScene::SpriteDraw()
{
	// ライトデータを一括でライトバッファに書き込む

	//GPUに送るためのLightDataベクターを作成
	std::vector<LightData> lightDataVector;

	for (auto& light : lights_) {
		LightData data{};
		light->WriteToLightData(data);
		lightDataVector.push_back(data);
	}

	lightsBufferData_->lightCount = static_cast<uint32_t>(lights_.size());
	memcpy(lightsBufferData_->lights, lightDataVector.data(), sizeof(LightData) * lightsBufferData_->lightCount);

	//directionalLight用のCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, lightsBufferResource_->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());

	for (auto& digitSprite : timerDigitSprites_) {
		digitSprite->Draw();
	}

}

void GameScene::ModelDraw()
{
	//ブロックの描画
	/*for (std::vector<Vector3*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (Vector3* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			blocks_.at(0)->SetTranslate(*worldTransformBlock);
			blocks_.at(0)->Draw();
		}
	}*/

	for (auto& block : blocks_) {
		block->Draw();
	}

	player_->Draw();

	for (auto& enemy : enemies_) {
		enemy->Draw();
	}

	for (auto& coin : coins_) {
		coin->Draw();
	}

	flag_->Draw();
}

void GameScene::SkinnedModelDraw()
{

}

void GameScene::ImGuiDraw()
{
	ImGui::SetNextWindowPos(ImVec2(20.0f, 82.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.55f);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs;

	ImGui::Begin("CoinCounter", nullptr, flags);
	ImGui::SetWindowFontScale(1.5f);
	ImGui::Text(
		"COINS  %u / %u",
		collectedCoinCount_,
		totalCoinCount_);
	ImGui::End();
}

void GameScene::Finalize()
{

}

void GameScene::GenerateBlocks()
{
	// 要素数
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定(縦方向のブロック数)
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 1列の要素数を設定(横方向のブロック数)
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	
	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				std::unique_ptr<Vector3> worldTransform = std::make_unique<Vector3>();
				worldTransformBlocks_[i][j] = move(worldTransform);
				*worldTransformBlocks_[i][j] = mapChipField_->GetMapChipPositionByIndex(j, i);
				std::unique_ptr<Object3D> object = std::make_unique<Object3D>();
				object->Initialize(object3dBasic_, modelManager_, "resources/Block.obj");
				object->SetTranslate(mapChipField_->GetMapChipPositionByIndex(j, i));
				blocks_.push_back(move(object));
			}
		}
	}

}

void GameScene::GenerateLevelObjects()
{
	LevelLoader levelLoader;
	levelData_ = levelLoader.LoadLevel("game_stage01");

	// レベルデータからオブジェクトを生成、配置
	for (auto& objectData : levelData_->objects) {

		modelManager_->LoadModel("resources", objectData.fileName + ".obj");

		// モデルを指定して3Dオブジェクトを生成
		std::unique_ptr<Object3D> newObject = std::make_unique<Object3D>();
		newObject->Initialize(
			object3dBasic_,
			modelManager_,
			"resources/" + objectData.fileName + ".obj"
		);

		// 座標
		newObject->SetTranslate(objectData.transform.translate);
		// 回転角
		newObject->SetRotate(objectData.transform.rotate);
		// スケール
		newObject->SetScale(objectData.transform.scale);
		// 配列に登録
		levelObjects_.push_back(std::move(newObject));
	}

	// プレイヤーは一データからプレイヤーを配置
	if (!levelData_->players.empty()) {
		auto& playerData = levelData_->players[0];
		playerSpawnPosition_ = playerData.translation;
		playerModel_->SetTranslate(playerData.translation);
		playerModel_->SetRotate(playerData.rotation);
	}

	for (const auto& enemyData : levelData_->enemies) {
		std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>();
		enemy->Initialize(object3dBasic_, modelManager_, enemyData.translation);
		enemies_.push_back(std::move(enemy));
	}

	collectedCoinCount_ = 0;
	for (const auto& coinData : levelData_->coins) {
		std::unique_ptr<Coin> coin = std::make_unique<Coin>();
		coin->Initialize(object3dBasic_, modelManager_, coinData.translation);
		coins_.push_back(std::move(coin));
	}
	totalCoinCount_ = static_cast<uint32_t>(coins_.size());
}

void GameScene::RespawnPlayer()
{
	player_->Respawn(playerSpawnPosition_);
	cameraController_->Reset();
	camera->Update();
	cameraForGPUData->worldPosition = camera->GetTranslate();
}

void GameScene::InitializeTimerDisplay()
{
	constexpr float kDigitSize = 48.0f;
	constexpr float kStartX = 20.0f;
	constexpr float kStartY = 20.0f;
	constexpr float kDigitSpacing = 4.0f;
	constexpr float kGroupSpacing = 12.0f;

	for (size_t digit = 0; digit < timerDigitTexturePaths_.size(); ++digit) {
		timerDigitTexturePaths_[digit] =
			"resources/numFont/" + std::to_string(digit) + ".png";
		textureManager_->LoadTexture(timerDigitTexturePaths_[digit]);
	}

	float positionX = kStartX;
	for (size_t digitIndex = 0; digitIndex < timerDigitSprites_.size(); ++digitIndex) {
		timerDigitSprites_[digitIndex] = std::make_unique<Sprite>();
		timerDigitSprites_[digitIndex]->Initialize(
			spriteBasic_,
			textureManager_,
			timerDigitTexturePaths_[0]);
		timerDigitSprites_[digitIndex]->SetPosition({ positionX, kStartY });
		timerDigitSprites_[digitIndex]->SetSize({ kDigitSize, kDigitSize });
		timerDigitSprites_[digitIndex]->SetIsDraw(true);

		positionX += kDigitSize + kDigitSpacing;
		if (digitIndex == 1 || digitIndex == 3) {
			positionX += kGroupSpacing;
		}
	}

	elapsedTime_ = 0.0f;
	UpdateTimerDisplay();
}

void GameScene::UpdateTimerDisplay()
{
	constexpr uint32_t kCentisecondsPerSecond = 100;
	constexpr uint32_t kSecondsPerMinute = 60;
	constexpr uint32_t kMaxCentiseconds =
		(99 * kSecondsPerMinute + 59) * kCentisecondsPerSecond + 99;

	uint32_t totalCentiseconds =
		static_cast<uint32_t>(elapsedTime_ * kCentisecondsPerSecond);
	if (totalCentiseconds > kMaxCentiseconds) {
		totalCentiseconds = kMaxCentiseconds;
	}

	const uint32_t minutes =
		totalCentiseconds / (kSecondsPerMinute * kCentisecondsPerSecond);
	const uint32_t seconds =
		(totalCentiseconds / kCentisecondsPerSecond) % kSecondsPerMinute;
	const uint32_t centiseconds = totalCentiseconds % kCentisecondsPerSecond;
	const std::array<uint32_t, kTimerDigitCount> digits = {
		minutes / 10,
		minutes % 10,
		seconds / 10,
		seconds % 10,
		centiseconds / 10,
		centiseconds % 10,
	};

	for (size_t digitIndex = 0; digitIndex < timerDigitSprites_.size(); ++digitIndex) {
		timerDigitSprites_[digitIndex]->SetTextureFilePath(
			timerDigitTexturePaths_[digits[digitIndex]]);
		timerDigitSprites_[digitIndex]->Update();
	}
}
