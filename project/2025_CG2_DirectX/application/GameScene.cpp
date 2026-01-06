#include "GameScene.h"
#include "../engine/Light/DirectionalLight.h"
#include "../engine/Camera/CameraForGPU.h"
#include "../engine/Light/PointLight.h"
#include "../engine/Light/SpotLight.h"
#include <numbers>
#include "ClearScene.h"
#include "../engine/Scene/SceneManager.h"

using namespace MyMath;

void GameScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Input* input, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
{
	directXBasic_ = directXBasic;
	object3dBasic_ = object3dBasic;
	modelManager_ = modelManager;
	input_ = input;
	logger_ = logger;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
	spriteBasic_ = spriteBasic;
	xaudio2Basic_ = xaudio2Basic;
	randomEngine_ = randomEngine;

	modelManager->LoadModel("resources", "player.obj");
	modelManager->LoadModel("resources", "Block.obj");
	modelManager->LoadModel("resources", "flag.obj");

	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	object3dBasic_->SetDefaultCamera(camera.get());

	flag_ = std::make_unique<Object3D>();
	flag_->Initialize(object3dBasic_, modelManager_, "resources/flag.obj");
	flag_->SetTranslate(mapChipField_->GetMapChipPositionByIndex(196, 18));
	flag_->SetRotate({ 0.0f, DEGtoRAD(90.0f), 0.0f });

	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	GenerateBlocks();

	//DirectionalLight用のリソースを作る
	directionalLightResource = directXBasic->CreateBufferResource(sizeof(DirectionalLight));
	//データを書き込む
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//デフォルト値はとりあえず以下のようにしておく
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// カメラ位置転送用のリソースを作る
	cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	// pointLightリソース
	pointLightResource = directXBasic->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->position = {};
	pointLightData->decay = 1.0f;
	pointLightData->radius = 30.0f;

	// SpotLightリソース
	spotLightResource = directXBasic->CreateBufferResource(sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->decay = 2.0f;
	spotLightData->direction = Vector3{ -1.0f, -1.0f, 0.0f }.Normalize();
	spotLightData->distance = 7.0f;
	spotLightData->intensity = 4.0f;
	spotLightData->position = { 2.0f, 1.25f, 0.0f };

	playerModel_ = std::make_unique<Object3D>();
	playerModel_->Initialize(object3dBasic_, modelManager_, "resources/player.obj");
	playerModel_->SetTranslate(mapChipField_->GetMapChipPositionByIndex(3, 18));

	player_ = std::make_unique<Player>();
	player_->Initialize(object3dBasic_, modelManager_, playerModel_.get(), input_, camera.get());
	player_->SetMapChipField(mapChipField_.get());

	//カメラコントローラの生成
	cameraController_ = std::make_unique<CameraController>();
	//カメラコントローラの初期化
	cameraController_->Initialize(camera.get());
	//追従対象をセット
	cameraController_->SetTarget(player_.get());
	cameraController_->Reset();

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
	//directionalLight用のCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());

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
