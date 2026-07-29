#include "Game.h"

void Game::Initialize()
{

	Engine::Initialize();

	currentScene_ = Scene::TitleScene;

	titleScene = std::make_unique<TitleScene>();
	sceneManager_->SetNextScene(move(titleScene));

}

void Game::Finalize()
{
	Engine::Finalize();
}

void Game::Update()
{
	imguiManager_->UpdateBegin();

	Engine::Update();

#ifdef _DEBUG
	if (Input::GetInstance()->IsTriggerKey(DIK_V)) {
		useDebugcamera_ = !useDebugcamera_;
	}

	//if (useDebugcamera) {
	//	debugcamera->Update(key);
	//}
#endif

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
