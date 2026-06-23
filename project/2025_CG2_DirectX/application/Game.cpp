#include "Game.h"

void Game::Initialize()
{

	Engine::Initialize();

	currentScene_ = Scene::SampleScene;

	sampleScene = std::make_unique<SampleScene>();
	sceneManager_->SetNextScene(move(sampleScene));

	/*titleScene = std::make_unique<TitleScene>();
	sceneManager_->SetNextScene(move(titleScene));*/

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

	//ゲームの処理

	imguiManager_->UpdateEnd();

	Draw();

}

void Game::Draw()
{
	Engine::PreDraw();
	Engine::SpritePreDraw();

	Engine::ModelPreDraw();

	Engine::BackBufferPreDraw();
	Engine::DrawRenderTexture();
	imguiManager_->Draw();

	Engine::PostDraw();
}
