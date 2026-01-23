#include "ClearScene.h"
#include "../engine/Light/DirectionalLight.h"
#include "../engine/Camera/CameraForGPU.h"
#include "../engine/Light/PointLight.h"
#include "../engine/Light/SpotLight.h"
#include <numbers>
#include "TitleScene.h"
#include "../engine/Scene/SceneManager.h"

using namespace MyMath;

void ClearScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
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

	modelManager->LoadModel("resources", "clear.obj");

	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	object3dBasic_->SetDefaultCamera(camera.get());

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

	object_ = std::make_unique<Object3D>();
	object_->Initialize(object3dBasic_, modelManager_, "resources/clear.obj");
	object_->SetRotate({ DEGtoRAD(90.0f), DEGtoRAD(180.0f), 0.0f });

}

void ClearScene::Update()
{

	if (Input::GetInstance()->IsTriggerPushKey(DIK_SPACE)) {
		std::unique_ptr<BaseScene> scene = std::make_unique<TitleScene>();
		sceneManager_->SetNextScene(move(scene));
	}

	camera->Update();
	object_->Update();

	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく
}

void ClearScene::SpriteDraw()
{
	//directionalLight用のCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());

}

void ClearScene::ModelDraw()
{
	object_->Draw();
}

void ClearScene::Finalize()
{
	
}
