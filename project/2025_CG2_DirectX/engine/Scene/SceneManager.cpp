#include "SceneManager.h"
#include "BaseScene.h"

#include <cassert>
#include <utility>

SceneManager::SceneManager() = default;

SceneManager::~SceneManager()
{
	// もしシーンが存在する場合は終了処理を行う
	FinalizeCurrentScene();
}

void SceneManager::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, SkinnedObject3DBasic* skinnedObject3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine, PostEffectController* postEffectController)
{
	// 引数のポインタが有効か確認
	assert(directXBasic);
	assert(object3dBasic);
	assert(skinnedObject3dBasic);
	assert(modelManager);
	assert(logger);
	assert(srvManager);
	assert(textureManager);
	assert(spriteBasic);
	assert(xaudio2Basic);
	assert(randomEngine);
	assert(postEffectController);

	// メンバ変数に代入
	directXBasic_ = directXBasic;
	object3dBasic_ = object3dBasic;
	skinnedObject3dBasic_ = skinnedObject3dBasic;
	modelManager_ = modelManager;
	logger_ = logger;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
	spriteBasic_ = spriteBasic;
	xaudio2Basic_ = xaudio2Basic;
	randomEngine_ = randomEngine;
	postEffectController_ = postEffectController;
}

void SceneManager::Update()
{

	ActivatePendingScene();

	if (scene_) {
		scene_->Update();
	}

}

void SceneManager::SpriteDraw()
{
	if (scene_) {
		scene_->SpriteDraw();
	}
}

void SceneManager::ModelDraw()
{
	if (scene_) {
		scene_->ModelDraw();
	}
}

void SceneManager::SkinnedModelDraw()
{
	if (scene_) {
		scene_->SkinnedModelDraw();
	}
}

void SceneManager::ImGuiDraw()
{
	if (scene_) {
		scene_->ImGuiDraw();
	}
}

void SceneManager::RequestSceneChange(std::unique_ptr<BaseScene> nextScene)
{
	assert(nextScene);
	pendingScene_ = std::move(nextScene);
}

void SceneManager::ActivatePendingScene()
{
	if (!pendingScene_) {
		return;
	}

	FinalizeCurrentScene();

	scene_ = std::move(pendingScene_);

	scene_->SetSceneManager(this);
	scene_->SetPostEffectController(postEffectController_);

	scene_->Initialize(
		directXBasic_,
		object3dBasic_,
		skinnedObject3dBasic_,
		modelManager_,
		logger_,
		srvManager_,
		textureManager_,
		spriteBasic_,
		xaudio2Basic_,
		randomEngine_);
}

void SceneManager::FinalizeCurrentScene()
{
	if (!scene_) {
		return;
	}

	scene_->Finalize();
	scene_.reset();
}

