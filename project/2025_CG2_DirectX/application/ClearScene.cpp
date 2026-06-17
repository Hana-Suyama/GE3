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

	// カメラ位置転送用のリソースを作る
	cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	object_ = std::make_unique<Object3D>();
	object_->Initialize(object3dBasic_, modelManager_, "resources/clear.obj");
	object_->SetRotate({ DEGtoRAD(90.0f), DEGtoRAD(180.0f), 0.0f });

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

void ClearScene::ModelDraw()
{
	object_->Draw();
}

void ClearScene::ImGuiDraw()
{
}

void ClearScene::Finalize()
{
	
}
