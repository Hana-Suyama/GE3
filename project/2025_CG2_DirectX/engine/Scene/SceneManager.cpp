#include "SceneManager.h"

SceneManager::~SceneManager()
{
	scene_->Finalize();
	scene_.reset();
	scene_ = nullptr;
}

void SceneManager::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
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
}

void SceneManager::Update()
{

	if (nextScene_) {
		if (scene_) {
			scene_->Finalize();
			scene_.reset();
			scene_ = nullptr;
		}

		scene_ = move(nextScene_);
		nextScene_ = nullptr;

		scene_->SetSceneManager(this);

		scene_->Initialize(directXBasic_, object3dBasic_, modelManager_, logger_, srvManager_, textureManager_, spriteBasic_, xaudio2Basic_, randomEngine_);
	}

	scene_->Update();

}

void SceneManager::SpriteDraw()
{
	scene_->SpriteDraw();
}

void SceneManager::ModelDraw()
{
	scene_->ModelDraw();
}

void SceneManager::ImGuiDraw()
{
	scene_->ImGuiDraw();
}

