#include "Game.h"

#include "TitleScene.h"

#include "SceneManager.h"
#include "ImGuiManager.h"
#include "Input.h"

#include <memory>

void Game::Initialize()
{

	Engine::Initialize();

	sceneManager_->RequestSceneChange(std::make_unique<TitleScene>());

}

void Game::Finalize()
{
	Engine::Finalize();
}

void Game::Update()
{
	imguiManager_->UpdateBegin();

	Engine::Update();

	sceneManager_->ImGuiDraw();
	Engine::PostEffectDebugDraw();

	//ゲームの処理

	imguiManager_->UpdateEnd();

	Draw();

}

void Game::Draw()
{
	Engine::RenderTexturePreDraw();
	Engine::SceneSpriteDraw();
	Engine::SceneModelDraw();

	Engine::BackBufferPreDraw();
	Engine::DrawPostEffect();
	imguiManager_->Draw();

	Engine::PostDraw();
}
