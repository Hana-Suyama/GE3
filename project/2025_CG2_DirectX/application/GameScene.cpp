#include "GameScene.h"

void GameScene::Initialize(Object3DBasic* object3dBasic, ModelManager* modelManager, Input* input, Camera* camera)
{
	object3dBasic_ = object3dBasic;
	modelManager_ = modelManager;
	input_ = input;
	camera_ = camera;

	/*for (int32_t i = 0; i < 60; i++) {
		Object3D* newblocks_ = new Object3D();
		newblocks_->Initialize(object3dBasic_, modelManager_, "resources/Block.obj");
		newblocks_->SetTranslate({ -30 + (i * 1.0f), 0.0f, 0.0f });
		blocks_.push_back(newblocks_);
	}*/

	playerModel_ = new Object3D();
	playerModel_->Initialize(object3dBasic_, modelManager_, "resources/player.obj");
	playerModel_->SetTranslate({ 0.0f, 1.0f, 0.0f });

	player_ = new Player();
	player_->Initialize(object3dBasic_, modelManager_, playerModel_, input_, camera_);

}

void GameScene::Update()
{
	for (Object3D* block : blocks_) {
		block->Update();
	}

	player_->Update();
}

void GameScene::Draw()
{
	for (Object3D* block : blocks_) {
		block->Draw();
	}

	player_->Draw();
}

void GameScene::Finalize()
{
	for (Object3D* block : blocks_) {
		delete block;
	}
	blocks_.clear();

	delete playerModel_;
	delete player_;
}
