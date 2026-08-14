#include "TitleScene.h"

#include "../engine/Light/DirectionalLight.h"
#include "../engine/Light/PointLight.h"
#include "../engine/Light/SpotLight.h"
#include "../engine/Scene/SceneManager.h"
#include "../engine/WindowsApi.h"
#include "GameScene.h"

void TitleScene::Initialize(
	DirectXBasic* directXBasic, Object3DBasic* object3dBasic,
	SkinnedObject3DBasic*, ModelManager*, Logger*,
	SRVManager*, TextureManager* textureManager, SpriteBasic* spriteBasic,
	XAudio2Basic*, std::mt19937*)
{
	directXBasic_ = directXBasic;

	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });
	camera_->Update();
	object3dBasic->SetDefaultCamera(camera_.get());

	cameraForGPUResource_ =
		directXBasic_->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(
		0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));
	cameraForGPUData_->worldPosition = camera_->GetTranslate();

	lights_.push_back(std::make_unique<DirectionalLight>());
	lights_.back()->Initialize();
	lights_.push_back(std::make_unique<PointLight>());
	lights_.back()->Initialize();
	lights_.push_back(std::make_unique<SpotLight>());
	lights_.back()->Initialize();

	lightsBufferResource_ =
		directXBasic_->CreateBufferResource(sizeof(LightBuffer));
	lightsBufferResource_->Map(
		0, nullptr, reinterpret_cast<void**>(&lightsBufferData_));

	const std::string titleLogoTexturePath = "resources/TitleLogo.png";
	textureManager->LoadTexture(titleLogoTexturePath);

	titleLogoSprite_ = std::make_unique<Sprite>();
	titleLogoSprite_->Initialize(
		spriteBasic, textureManager, titleLogoTexturePath);
	titleLogoSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	titleLogoSprite_->SetPosition({
		static_cast<float>(WindowsApi::kClientWidth) * 0.5f,
		static_cast<float>(WindowsApi::kClientHeight) * 0.5f,
	});
	titleLogoSprite_->SetIsDraw(true);
	titleLogoSprite_->Update();
}

void TitleScene::Update()
{
	if (Input::GetInstance()->IsTriggerPushKey(DIK_SPACE)) {
		std::unique_ptr<BaseScene> scene = std::make_unique<GameScene>();
		sceneManager_->SetNextScene(std::move(scene));
	}

	camera_->Update();
	cameraForGPUData_->worldPosition = camera_->GetTranslate();
	titleLogoSprite_->Update();
}

void TitleScene::SpriteDraw()
{
	std::vector<LightData> lightDataVector;
	lightDataVector.reserve(lights_.size());
	for (auto& light : lights_) {
		LightData data{};
		light->WriteToLightData(data);
		lightDataVector.push_back(data);
	}

	lightsBufferData_->lightCount = static_cast<uint32_t>(lights_.size());
	memcpy(
		lightsBufferData_->lights,
		lightDataVector.data(),
		sizeof(LightData) * lightsBufferData_->lightCount);

	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(
		3, lightsBufferResource_->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(
		4, cameraForGPUResource_->GetGPUVirtualAddress());

	titleLogoSprite_->Draw();
}

void TitleScene::ModelDraw()
{
}

void TitleScene::SkinnedModelDraw()
{
}

void TitleScene::ImGuiDraw()
{
}

void TitleScene::Finalize()
{
}
