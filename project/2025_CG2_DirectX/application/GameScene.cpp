#include "GameScene.h"
#include "../engine/Light/DirectionalLight.h"
#include "../engine/Camera/CameraForGPU.h"
#include "../engine/Light/PointLight.h"
#include "../engine/Light/SpotLight.h"
#include <numbers>
#include "ClearScene.h"
#include "../engine/Scene/SceneManager.h"

using namespace MyMath;

void GameScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
{
	directXBasic_ = directXBasic;
	object3dBasic_ = object3dBasic;
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

	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");

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
	playerModel_->SetTranslate(mapChipField_->GetMapChipPositionByIndex(3, 18));

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

	if (isClear_) {
		std::unique_ptr<BaseScene> scene = std::make_unique<ClearScene>();
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
	flag_->Update();

	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	if (playerModel_->GetTransform().translate.y <= -5.0f) {
		playerModel_->SetTranslate(mapChipField_->GetMapChipPositionByIndex(3, 18));
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
	flag_->Draw();
}

void GameScene::ImGuiDraw()
{
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
		playerModel_->SetTranslate(playerData.translation);
		playerModel_->SetRotate(playerData.rotation);
	}

}
