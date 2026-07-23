#include "SampleScene.h"
#include <numbers>
#include "../../engine/Utility/Math/Lerp.h"
#include <TimeManager.h>
#include <AreaLight.h>
#include <PostEffectController.h>
#include "../../../AnimationManager.h"
using namespace MyMath;

void SampleScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, SkinnedObject3DBasic* skinnedObject3dBasic, ModelManager* modelManager, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
{
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

	textureManager_->LoadTexture("resources/uvChecker.png");
	textureManager_->LoadTexture("resources/monsterBall.png");
	textureManager_->LoadTexture("resources/particle.png");
	//textureManager_->AllIntermediateResourceRelease();
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");

	modelManager_->LoadModelAssimp("resources", "plane.obj");
	modelManager_->LoadModelAssimp("resources", "teapot.obj");
	modelManager_->LoadModel("resources", "fence.obj");
	modelManager_->LoadModel("resources", "multiMesh.obj");
	modelManager_->LoadModel("resources", "multiMaterial.obj");
	modelManager_->LoadModel("resources", "bunny.obj");
	modelManager_->LoadModel("resources", "suzanne.obj");
	modelManager_->LoadModel("resources", "terrain.obj");
	modelManager_->LoadModelAssimp("resources", "plane.gltf");
	modelManager_->CreateSphere();
	modelManager_->CreateSkyBox();
	modelManager_->LoadModelAssimp("resources", "player.obj");
	modelManager_->LoadModelAssimp("resources", "enemy.obj");
	modelManager_->LoadModelAssimp("resources", "AnimatedCube.gltf");
	modelManager_->LoadModelAssimp("resources/human", "walk.gltf");
	animation = AnimationManager::LoadAnimetionFile("resources", "AnimatedCube.gltf");
	walkAnimation = AnimationManager::LoadAnimetionFile("resources/human", "walk.gltf");

	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	object3dBasic_->SetDefaultCamera(camera.get());
	skinnedObject3dBasic_->SetDefaultCamera(camera.get());

	particleManager = std::make_unique<ParticleManager>();
	particleManager->Initialize(directXBasic, srvManager, logger, textureManager, "resources/particle.png", camera.get());

	skyBoxBasic_ = std::make_unique<SkyBoxBasic>();
	skyBoxBasic_->Initialize(directXBasic, logger);
	skyBoxBasic_->SetDefaultCamera(camera.get());

	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(skyBoxBasic_.get(), modelManager, "debug_skybox");

	//sprite
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(spriteBasic, textureManager, "resources/uvChecker.png");

	sprite2 = std::make_unique<Sprite>();
	sprite2->Initialize(spriteBasic, textureManager, "resources/monsterBall.png");

	object3d = std::make_unique<Object3D>();
	object3d->Initialize(object3dBasic_, modelManager, "resources/plane.obj");

	object3dTeapot = std::make_unique<Object3D>();
	object3dTeapot->Initialize(object3dBasic_, modelManager, "resources/teapot.obj");

	object3dMultiMesh = std::make_unique<Object3D>();
	object3dMultiMesh->Initialize(object3dBasic_, modelManager, "resources/multiMesh.obj");

	object3dMultiMaterial = std::make_unique<Object3D>();
	object3dMultiMaterial->Initialize(object3dBasic_, modelManager, "resources/multiMaterial.obj");

	object3dBunny = std::make_unique<Object3D>();
	object3dBunny->Initialize(object3dBasic_, modelManager, "resources/bunny.obj");

	object3dSuzanne = std::make_unique<Object3D>();
	object3dSuzanne->Initialize(object3dBasic_, modelManager, "resources/suzanne.obj");

	object3dTerrain = std::make_unique<Object3D>();
	object3dTerrain->Initialize(object3dBasic_, modelManager, "resources/terrain.obj");

	object3dPlanegLTF = std::make_unique<Object3D>();
	object3dPlanegLTF->Initialize(object3dBasic_, modelManager, "resources/plane.gltf");

	object3dSphere = std::make_unique<Object3D>();
	object3dSphere->Initialize(object3dBasic_, modelManager, "debug_sphere");

	object3dPlayer = std::make_unique<Object3D>();
	object3dPlayer->Initialize(object3dBasic_, modelManager, "resources/player.obj");

	object3dAnimCube = std::make_unique<Object3D>();
	object3dAnimCube->Initialize(object3dBasic_, modelManager, "resources/AnimatedCube.gltf");

	object3dWalk = std::make_unique<SkinnedObject3D>();
	object3dWalk->Initialize(skinnedObject3dBasic_, modelManager, "resources/human/walk.gltf");
	object3dWalk->SetAnimation(walkAnimation);

	// カメラ位置転送用のリソースを作る
	cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	lights_.push_back(std::make_unique<DirectionalLight>());
	lights_.back()->Initialize();

	lights_.push_back(std::make_unique<AreaLight>());
	lights_.back()->Initialize();

	lights_.push_back(std::make_unique<SpotLight>());
	lights_.back()->Initialize();
	lights_.push_back(std::make_unique<SpotLight>());
	lights_.back()->Initialize();

	// ライトを一括でGPUに送るためのバッファを作る
	size_t alignedSize = (sizeof(LightBuffer) + 255) & ~255;
	lightsBufferResource_ = directXBasic->CreateBufferResource(alignedSize);
	// データを書き込む
	lightsBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightsBufferData_));

	GenerateLevelObjects();

}

void SampleScene::Update()
{
	
	camera->Update();

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	particleManager->Update(EmitterPosition, *randomEngine_);

	sprite->Update();
	sprite2->Update();

	object3d->Update();
	object3dTeapot->Update();
	object3dMultiMesh->Update();
	object3dMultiMaterial->Update();
	object3dBunny->Update();
	object3dSuzanne->Update();
	object3dTerrain->Update();
	object3dPlanegLTF->Update();
	object3dSphere->Update();
	object3dAnimCube->Update();
	object3dPlayer->Update();
	object3dWalk->Update();

	for (auto& object : levelObjects_) {
		object->Update();
	}

	for (auto& enemy : enemyObjects_) {
		enemy->Update();
	}

	skyBox_->Update();

	for (auto& light : lights_) {
		light->Update();
	}

	if (playSound) {
		//音声再生
		xaudio2Basic_->PlayAudio("resources/Alarm01.wav");
		playSound = false;
	}

	if (Input::GetInstance()->IsPadButtonDown(XINPUT_GAMEPAD_A))
	{
		Input::GetInstance()->PlayVibration(1.0f, 0.3f, 0.2f);
	}

	for (BYTE key : Input::GetInstance()->GetTriggerPushKeys()) {
		if (!postEffectController_) {
			break;
		}

		switch (key) {
		case DIK_1:
			postEffectController_->SetType(PostEffectType::None);
			break;
		case DIK_2:
			postEffectController_->SetType(PostEffectType::Grayscale);
			break;
		case DIK_3:
			postEffectController_->SetType(PostEffectType::Sepia);
			break;
		case DIK_4:
			postEffectController_->SetType(PostEffectType::Vignette);
			break;
		case DIK_5:
			postEffectController_->SetType(PostEffectType::BoxFilter3x3);
			break;
		case DIK_6:
			postEffectController_->SetType(PostEffectType::BoxFilter5x5);
			break;
		case DIK_7:
			postEffectController_->SetType(PostEffectType::GaussianBlur);
			break;
		case DIK_8:
			postEffectController_->SetType(PostEffectType::RadialBlur);
			break;
		case DIK_9:
			postEffectController_->SetType(PostEffectType::LuminanceOutline);
			break;
		case DIK_Q:
			postEffectController_->SetType(PostEffectType::DepthOutline);
			break;
		case DIK_W:
			postEffectController_->SetType(PostEffectType::Random);
			break;
		case DIK_E:
			postEffectController_->SetType(PostEffectType::Dissolve);
			break;
		default:
			break;
		}
	}

	if (postEffectController_->GetType() == PostEffectType::Dissolve) {
		if (Input::GetInstance()->IsPushKey(DIK_LEFTARROW)) {
			DissolveThreshold -= 0.01f;
		}
		if (Input::GetInstance()->IsPushKey(DIK_RIGHTARROW)) {
			DissolveThreshold += 0.01f;
		}
		DissolveThreshold = std::clamp(DissolveThreshold, 0.0f, 1.0f);
		postEffectController_->SetThreshold(DissolveThreshold);
	}

	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

}

void SampleScene::SpriteDraw()
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

	
	sprite->Draw();
	sprite2->Draw();
}

void SampleScene::ModelDraw()
{
	//directionalLight用のCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, lightsBufferResource_->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());

	object3d->Draw();
	object3dTeapot->Draw();
	object3dMultiMesh->Draw();
	object3dMultiMaterial->Draw();
	object3dBunny->Draw();
	object3dSuzanne->Draw();
	object3dTerrain->Draw();
	object3dPlanegLTF->Draw();
	object3dSphere->Draw();
	object3dAnimCube->Draw();
	object3dPlayer->Draw();
	
	//object3dWalk->DrawSkeletonDebug();

	for (auto& object : levelObjects_) {
		//object->Draw();
	}

	for (auto& enemy : enemyObjects_) {
		//enemy->Draw();
	}

	particleManager->Draw();

	skyBoxBasic_->SkyBoxPreDraw();
	skyBox_->Draw();

}

void SampleScene::SkinnedModelDraw()
{
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, lightsBufferResource_->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());

	object3dWalk->Draw();
}

void SampleScene::ImGuiDraw()
{
#ifdef USE_IMGUI
	////開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
	ImGui::Begin("ImGui");

	ImGui::Text("Application average %.1f FPS", ImGui::GetIO().Framerate);

	sprite->DebugDraw("Sprite 1");
	sprite2->DebugDraw("Sprite 2");

	object3d->DebugDraw("plane");
	object3dTeapot->DebugDraw("teapot");
	object3dBunny->DebugDraw("Bunny");
	object3dMultiMesh->DebugDraw("MultiMesh");
	object3dMultiMaterial->DebugDraw("MultiMaterial");
	object3dSuzanne->DebugDraw("Suzanne");
	object3dTerrain->DebugDraw("Terrain");
	object3dPlanegLTF->DebugDraw("planegltf");
	object3dSphere->DebugDraw("Sphere");
	object3dAnimCube->DebugDraw("AnimatedCube");
	object3dPlayer->DebugDraw("Player");
	object3dWalk->DebugDraw("Walk");

	skyBox_->DebugDraw("SkyBox");

	if (ImGui::TreeNode("Camera")) {
		ImGui::DragFloat3("Rotate", reinterpret_cast<float*>(&cameraTransform.rotate), 0.1f, -30.0f, 30.0f);
		ImGui::DragFloat3("Translate", reinterpret_cast<float*>(&cameraTransform.translate), 0.1f, -100.0, 100.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Light")) {
		for (int32_t i = 0; auto& light : lights_) {
			light->DebugDrawImGui(std::to_string(i));
			i++;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Sound")) {
		if (ImGui::Button("play")) {
			playSound = true;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Key")) {
		ImGui::Text("PushKey : %d", Input::GetInstance()->IsPushKey(DIK_SPACE));
		ImGui::Text("TriggerKey : %d", Input::GetInstance()->IsTriggerKey(DIK_SPACE));
		ImGui::Text("PadButton : A %d", Input::GetInstance()->IsPadButton(XINPUT_GAMEPAD_A));
		ImGui::Text("Gamepad RightJoy : %f %f", Input::GetInstance()->GetRightStick().x, Input::GetInstance()->GetRightStick().y);
		ImGui::Text("Gamepad LeftJoy : %f %f", Input::GetInstance()->GetLeftStick().x, Input::GetInstance()->GetLeftStick().y);
		ImGui::Text("Gamepad RightTrigger : %f", Input::GetInstance()->GetRightTrigger());
		ImGui::Text("Gamepad LeftTrigger : %f", Input::GetInstance()->GetLeftTrigger());
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Time")) {
		ImGui::Text("DeltaTime : %f", TimeManager::GetInstance()->GetDeltaTime());
		ImGui::Text("UnscaledDeltaTime : %f", TimeManager::GetInstance()->GetUnscaledDeltaTime());
		ImGui::Text("TimeScale : %f", TimeManager::GetInstance()->GetTimeScale());
		if (ImGui::Button("Scale 0.1")) {
			TimeManager::GetInstance()->SetTimeScale(0.1f);
		}
		ImGui::TreePop();
	}
	ImGui::End();
#endif
}

void SampleScene::Finalize()
{
	
}

void SampleScene::SetLight()
{
	
}

void SampleScene::GenerateLevelObjects()
{
	LevelLoader levelLoader;
	levelData_ = levelLoader.LoadLevel("stage01");

	// レベルデータからオブジェクトを生成、配置
	for (auto& objectData : levelData_->objects) {

		modelManager_->LoadModel("resources", objectData.fileName + ".obj");

		// モデルを指定して3Dオブジェクトを生成
		std::unique_ptr<Object3D> newObject = std::make_unique<Object3D>();
		newObject->Initialize(
			object3dBasic_,
			modelManager_,
			"resources/" + objectData.fileName + ".obj"
		);

		// 座標
		newObject->SetTranslate(objectData.transform.translate);
		// 回転角
		newObject->SetRotate(objectData.transform.rotate);
		// スケール
		newObject->SetScale(objectData.transform.scale);
		// 配列に登録
		levelObjects_.push_back(std::move(newObject));
	}

	// プレイヤーは一データからプレイヤーを配置
	if (!levelData_->players.empty()) {
		auto& playerData = levelData_->players[0];
		object3dPlayer->SetTranslate(playerData.translation);
		object3dPlayer->SetRotate(playerData.rotation);
	}

	for (auto& enemyData : levelData_->enemies) {
		// 敵の生成
		// モデルを指定して3Dオブジェクトを生成
		std::unique_ptr<Object3D> newEnemy = std::make_unique<Object3D>();
		newEnemy->Initialize(object3dBasic_, modelManager_, "resources/enemy.obj");
		newEnemy->SetTranslate(enemyData.translation);
		newEnemy->SetRotate(enemyData.rotation);
		// 敵リストに追加
		enemyObjects_.push_back(std::move(newEnemy));
	}
}
