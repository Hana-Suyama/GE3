#include "SampleScene.h"
#include <numbers>
using namespace MyMath;

void SampleScene::Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic, ModelManager* modelManager, Input* input, Logger* logger, SRVManager* srvManager, TextureManager* textureManager, SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic, std::mt19937* randomEngine)
{
	directXBasic_ = directXBasic;
	object3dBasic_ = object3dBasic;
	modelManager_ = modelManager;
	input_ = input;
	logger_ = logger;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
	spriteBasic_ = spriteBasic;
	xaudio2Basic_ = xaudio2Basic;
	randomEngine_ = randomEngine;

	camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	object3dBasic_->SetDefaultCamera(camera);

	particleManager = new ParticleManager();
	particleManager->Initialize(directXBasic, srvManager, logger, textureManager, "resources/particle.png", camera);

	//sprite
	sprite = new Sprite();
	sprite->Initialize(spriteBasic, textureManager, "resources/uvChecker.png");

	sprite2 = new Sprite();
	sprite2->Initialize(spriteBasic, textureManager, "resources/monsterBall.png");

	object3d = new Object3D();
	object3d->Initialize(object3dBasic_, modelManager, "resources/plane.obj");

	object3dTeapot = new Object3D();
	object3dTeapot->Initialize(object3dBasic_, modelManager, "resources/teapot.obj");

	object3dMultiMesh = new Object3D();
	object3dMultiMesh->Initialize(object3dBasic_, modelManager, "resources/multiMesh.obj");

	object3dMultiMaterial = new Object3D();
	object3dMultiMaterial->Initialize(object3dBasic_, modelManager, "resources/multiMaterial.obj");

	object3dBunny = new Object3D();
	object3dBunny->Initialize(object3dBasic_, modelManager, "resources/bunny.obj");

	object3dSuzanne = new Object3D();
	object3dSuzanne->Initialize(object3dBasic_, modelManager, "resources/suzanne.obj");

	object3dTerrain = new Object3D();
	object3dTerrain->Initialize(object3dBasic_, modelManager, "resources/terrain.obj");


	//DirectionalLight用のリソースを作る
	directionalLightResource = directXBasic->CreateBufferResource(sizeof(DirectionalLight));
	//データを書き込む
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//デフォルト値はとりあえず以下のようにしておく
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// カメラ位置転送用のリソースを作る
	cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	// pointLightリソース
	pointLightResource = directXBasic->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->position = {};
	pointLightData->decay = 1.0f;
	pointLightData->radius = 30.0f;

	// SpotLightリソース
	spotLightResource = directXBasic->CreateBufferResource(sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->decay = 2.0f;
	spotLightData->direction = Normalize({ -1.0f, -1.0f, 0.0f });
	spotLightData->distance = 7.0f;
	spotLightData->intensity = 4.0f;
	spotLightData->position = { 2.0f, 1.25f, 0.0f };

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

	if (playSound) {
		//音声再生
		xaudio2Basic_->PlayAudio("resources/Alarm01.wav");
		playSound = false;
	}

#ifdef USE_IMGUI
	////開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
	ImGui::Begin("ImGui");
	sprite->DebugDraw("Sprite 1");
	sprite2->DebugDraw("Sprite 2");

	object3d->DebugDraw("plane");
	///*if (ImGui::TreeNode("Sphere")) {
	//	ImGui::Checkbox("drawSphere", &drawSphere);
	//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSphere.scale), -5, 5);
	//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSphere.rotate), -5, 5);
	//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSphere.translate), -5, 5);
	//	ImGui::Combo("Ligting", &materialDataSphere->enableLighting, "None\0Lambert\0Half Lambert\0\0");
	//	ImGui::TreePop();
	//}*/
	object3dTeapot->DebugDraw("teapot");
	object3dBunny->DebugDraw("Bunny");
	object3dMultiMesh->DebugDraw("MultiMesh");
	object3dMultiMaterial->DebugDraw("MultiMaterial");
	object3dSuzanne->DebugDraw("Suzanne");
	object3dTerrain->DebugDraw("Terrain");
	if (ImGui::TreeNode("Camera")) {
		ImGui::DragFloat3("Rotate", reinterpret_cast<float*>(&cameraTransform.rotate), 0.1f, -30.0f, 30.0f);
		ImGui::DragFloat3("Translate", reinterpret_cast<float*>(&cameraTransform.translate), 0.1f, -100.0, 100.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lighting")) {
		ImGui::SliderFloat3("Direction", reinterpret_cast<float*>(&directionalLightData->direction), -1, 1);
		ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&directionalLightData->color));
		ImGui::SliderFloat("Intensity", &directionalLightData->intensity, 0.0f, 1.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("PointLighting")) {
		ImGui::SliderFloat3("position", reinterpret_cast<float*>(&pointLightData->position), -10, 10);
		ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&pointLightData->color));
		ImGui::SliderFloat("Intensity", &pointLightData->intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Radius", &pointLightData->radius, 0.0f, 100.0f);
		ImGui::SliderFloat("Decay", &pointLightData->decay, 0.0f, 30.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("SpotLighting")) {
		ImGui::SliderFloat3("position", reinterpret_cast<float*>(&spotLightData->position), -10, 10);
		ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&spotLightData->color));
		ImGui::SliderFloat("cosAngle", &spotLightData->cosAngle, 0.0f, 1.0f);
		ImGui::SliderFloat("decay", &spotLightData->decay, 0.0f, 1.0f);
		ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&spotLightData->direction), 0.0f, 1.0f);
		ImGui::SliderFloat("distance", &spotLightData->distance, 0.0f, 1.0f);
		ImGui::SliderFloat("intensity", &spotLightData->intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("cosFallOffStart", &spotLightData->cosFalloffStart, 0.0f, 1.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Sound")) {
		if (ImGui::Button("play")) {
			playSound = true;
		}
		ImGui::TreePop();
	}
	///*if (ImGui::TreeNode("Key")) {
	//	ImGui::Text("PushKey : %d", input->PushKey(DIK_SPACE));
	//	ImGui::Text("TriggerKey : %d", input->TriggerKey(DIK_SPACE));
	//	ImGui::Text("Gamepad RightJoy : %ld", input->GetPadKey().lRx);
	//	ImGui::Text("Gamepad RightJoy : %ld", ((input->GetPadKey().lRx - static_cast<LONG>(32767.0)) / static_cast <LONG>(10000.0)));
	//	ImGui::TreePop();
	//}*/
	ImGui::End();
#endif

	directionalLightData->direction = Normalize(directionalLightData->direction);
	spotLightData->direction = Normalize(spotLightData->direction);
	if (spotLightData->cosAngle == spotLightData->cosFalloffStart) {
		spotLightData->cosFalloffStart += 0.01f;
	}

	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

}

void SampleScene::SpriteDraw()
{
	//directionalLight用のCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());

	sprite->Draw();
	sprite2->Draw();
}

void SampleScene::ModelDraw()
{
	object3d->Draw();
	object3dTeapot->Draw();
	object3dMultiMesh->Draw();
	object3dMultiMaterial->Draw();
	object3dBunny->Draw();
	object3dSuzanne->Draw();
	object3dTerrain->Draw();

	particleManager->Draw();
}

void SampleScene::Finalize()
{
	delete object3dTerrain;
	delete object3dSuzanne;
	delete object3dBunny;
	delete object3dMultiMaterial;
	delete object3dMultiMesh;
	delete object3dTeapot;
	delete object3d;
	delete sprite2;
	delete sprite;
	delete particleManager;
}

void SampleScene::SetLight()
{
	
}

