#include "TitleScene.h"

#include "../engine/Scene/SceneManager.h"
#include "../engine/WindowsApi.h"
#include "GameScene.h"

void TitleScene::Initialize(
	DirectXBasic*, Object3DBasic*, SkinnedObject3DBasic*, ModelManager*, Logger*,
	SRVManager*, TextureManager* textureManager, SpriteBasic* spriteBasic,
	XAudio2Basic*, std::mt19937*)
{
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

	titleLogoSprite_->Update();
}

void TitleScene::SpriteDraw()
{
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
