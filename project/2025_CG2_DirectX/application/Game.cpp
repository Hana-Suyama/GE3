#include "Game.h"

void Game::Initialize()
{

	Engine::Initialize();

	textureManager->LoadTexture("resources/uvChecker.png");
	textureManager->LoadTexture("resources/monsterBall.png");
	textureManager->LoadTexture("resources/particle.png");

	modelManager->LoadModel("resources", "plane.obj");
	modelManager->LoadModel("resources", "teapot.obj");
	modelManager->LoadModel("resources", "fence.obj");
	modelManager->LoadModel("resources", "player.obj");
	modelManager->LoadModel("resources", "Block.obj");
	modelManager->LoadModel("resources", "multiMesh.obj");
	modelManager->LoadModel("resources", "multiMaterial.obj");
	modelManager->LoadModel("resources", "bunny.obj");
	modelManager->LoadModel("resources", "suzanne.obj");
	modelManager->LoadModel("resources", "terrain.obj");

	gameScene = new GameScene();
	gameScene->Initialize(object3DBasic, modelManager, input, defaultCamera);

	sampleScene = new SampleScene();
	sampleScene->Initialize(directXBasic, object3DBasic, modelManager, input, logger, srvManager, textureManager, spriteBasic, xaudio2Basic, &randomEngine);

	//音声読み込み
	xaudio2Basic->LoadSound("resources/Alarm01.wav");

}

void Game::Finalize()
{
	sampleScene->Finalize();
	delete sampleScene;
	gameScene->Finalize();
	delete gameScene;

	Engine::Finalize();
}

void Game::Update()
{

	Engine::Update();

	imguiManager->UpdateBegin();

#ifdef _DEBUG
	if (input->TriggerKey(DIK_V)) {
		useDebugcamera = !useDebugcamera;
	}

	//if (useDebugcamera) {
	//	debugcamera->Update(key);
	//}
#endif

	//ゲームの処理

	//gameScene->Update();
	sampleScene->Update();

	imguiManager->UpdateEnd();

	Draw();

}

void Game::Draw()
{
	Engine::PreDraw();
	Engine::SpritePreDraw();

	sampleScene->SpriteDraw();

	Engine::ModelPreDraw();

	//gameScene->Draw();
	sampleScene->ModelDraw();

	imguiManager->Draw();

	Engine::PostDraw();
}
