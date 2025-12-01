#include "cmath"
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <vector>
#include <wrl.h>
#include "2025_CG2_DirectX/engine/Input.h"
#include "2025_CG2_DirectX/engine/utility/Math/MyMath.h"
#include "2025_CG2_DirectX/engine/Debug/DebugCamera.h"
#include "2025_CG2_DirectX/engine/WindowsApi.h"
#include "2025_CG2_DirectX/engine/DirectXBasic.h"
#include "2025_CG2_DirectX/engine/utility/Logger.h"
#include <dxcapi.h>
#include "2025_CG2_DirectX/engine/utility/StringUtility.h"
#include "2025_CG2_DirectX/engine/Sprite/SpriteBasic.h"
#include "VertexData.h"
#include "2025_CG2_DirectX/engine/Sprite/Sprite.h"
#include "2025_CG2_DirectX/engine/TextureManager.h"
#include "2025_CG2_DirectX/engine/Object3D/Object3DBasic.h"
#include "2025_CG2_DirectX/engine/Model/ModelManager.h"
#include "2025_CG2_DirectX/engine/Object3D/Object3D.h"
#include "2025_CG2_DirectX/engine/SRVManager.h"
#include "2025_CG2_DirectX/engine/Particle/ParticleManager.h"
#include "2025_CG2_DirectX/engine/debug/ImGui/ImGuiManager.h"
#include <random>
#include "2025_CG2_DirectX/engine/Audio/XAudio2Basic.h"
#include "2025_CG2_DirectX/application/GameScene.h"
using namespace MyMath;

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")

struct DirectionalLight {
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};

struct CameraForGPU {
	Vector3 worldPosition;
};

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		//リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	//時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリいかに出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	//processID(このexeのID)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	//設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	//Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	//他に関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

//Windowsアプリでのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;

	//COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	//誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	//loggerの初期化
	Logger* logger = nullptr;
	logger = new Logger();
	logger->Initialize();

	//起動時にログ出力のテスト
	logger->Log("test\n");

	//WindowsApi
	WindowsApi* winApi = nullptr;
	//WinApiの初期化
	winApi = new WindowsApi();
	winApi->Initialize();

	//DirectXの初期化
	DirectXBasic* directXBasic = nullptr;
	directXBasic = new DirectXBasic();
	directXBasic->Initialize(logger, winApi);

	SRVManager* srvManager = nullptr;
	srvManager = new SRVManager();
	srvManager->Initialize(directXBasic);

	ImGuiManager* imguiManager = nullptr;
	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApi, directXBasic, srvManager);

	//テクスチャマネージャの初期化
	TextureManager* textureManager = nullptr;
	textureManager = new TextureManager();
	textureManager->Initialize(directXBasic, srvManager);

	//モデルマネージャの初期化
	ModelManager* modelManager = nullptr;
	modelManager = new ModelManager();
	modelManager->Initialize(directXBasic, textureManager);

	XAudio2Basic* xaudio2Basic = nullptr;
	xaudio2Basic = new XAudio2Basic();
	xaudio2Basic->Initialize();

	//ポインタ
	Input* input = nullptr;
	//入力の初期化
	input = new Input();
	input->Initialize(winApi);
	
	logger->Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

	//スプライト基盤
	SpriteBasic* spriteBasic = nullptr;
	spriteBasic = new SpriteBasic();
	spriteBasic->Initialize(directXBasic, logger);

	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });

	//3Dオブジェクト基盤
	Object3DBasic* object3DBasic = nullptr;
	object3DBasic = new Object3DBasic();
	object3DBasic->Initialize(directXBasic, logger);

	object3DBasic->SetDefaultCamera(camera);

	//カメラ用変数を作る
	struct Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -30.0f } };

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

	GameScene* gameScene = nullptr;
	gameScene = new GameScene();
	gameScene->Initialize(object3DBasic, modelManager, input, camera);

	ParticleManager* particleManager = nullptr;
	particleManager = new ParticleManager();
	particleManager->Initialize(directXBasic, srvManager, logger, textureManager, "resources/particle.png", camera);


	//sprite
	Sprite* sprite = nullptr;
	sprite = new Sprite();
	sprite->Initialize(spriteBasic, textureManager, "resources/uvChecker.png");

	Sprite* sprite2 = nullptr;
	sprite2 = new Sprite();
	sprite2->Initialize(spriteBasic, textureManager, "resources/monsterBall.png");

	Object3D* object3d = nullptr;
	object3d = new Object3D();
	object3d->Initialize(object3DBasic, modelManager, "resources/plane.obj");

	Object3D* object3dTeapot = nullptr;
	object3dTeapot = new Object3D();
	object3dTeapot->Initialize(object3DBasic, modelManager, "resources/teapot.obj");

	Object3D* object3dMultiMesh = nullptr;
	object3dMultiMesh = new Object3D();
	object3dMultiMesh->Initialize(object3DBasic, modelManager, "resources/multiMesh.obj");

	Object3D* object3dMultiMaterial = nullptr;
	object3dMultiMaterial = new Object3D();
	object3dMultiMaterial->Initialize(object3DBasic, modelManager, "resources/multiMaterial.obj");

	Object3D* object3dBunny = nullptr;
	object3dBunny = new Object3D();
	object3dBunny->Initialize(object3DBasic, modelManager, "resources/bunny.obj");

	Object3D* object3dSuzanne = nullptr;
	object3dSuzanne = new Object3D();
	object3dSuzanne->Initialize(object3DBasic, modelManager, "resources/suzanne.obj");

	//DirectionalLight用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = directXBasic->CreateBufferResource(sizeof(DirectionalLight));
	//データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//デフォルト値はとりあえず以下のようにしておく
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// カメラ位置転送用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource = directXBasic->CreateBufferResource(sizeof(CameraForGPU));
	// データを書き込む
	CameraForGPU* cameraForGPUData = nullptr;
	// 書き込むためのアドレスを取得
	cameraForGPUResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData));
	cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

	//音声読み込み
	//SoundData soundData1 = xaudio2Basic->LoadSound("resources/Alarm01.wav");
	xaudio2Basic->LoadSound("resources/Alarm01.wav");
	xaudio2Basic->LoadSound("resources/Alarm01.wav");

	BYTE beforeKey[256] = {};
	
	DebugCamera* debugcamera = new DebugCamera();
	debugcamera->Initialize(WindowsApi::kClientWidth, WindowsApi::kClientHeight);
	bool useDebugcamera = false;

	bool playSound = false;

	Vector3 EmitterPosition{};

	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		
		//Windowsのメッセージ処理
		if (winApi->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		 
		imguiManager->UpdateBegin();

		input->Update();

		#ifdef _DEBUG
			if(input->TriggerKey(DIK_V)) {
				useDebugcamera = !useDebugcamera;
			}

			//if (useDebugcamera) {
			//	debugcamera->Update(key);
			//}
		#endif

		//ゲームの処理

		//gameScene->Update();


		camera->Update();


		particleManager->Update(EmitterPosition, randomEngine);

		sprite->Update();
		sprite2->Update();

		object3d->Update();
		object3dTeapot->Update();
		object3dMultiMesh->Update();
		object3dMultiMaterial->Update();
		object3dBunny->Update();
		object3dSuzanne->Update();

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

		camera->SetTranslate(cameraTransform.translate);
		camera->SetRotate(cameraTransform.rotate);

		if (playSound) {
			//音声再生
			xaudio2Basic->PlayAudio("resources/Alarm01.wav");
			playSound = false;
		}

		directionalLightData->direction = Normalize(directionalLightData->direction);

		cameraForGPUData->worldPosition = camera->GetTranslate();// あとでワールド座標取得に変えておく

		imguiManager->UpdateEnd();

		directXBasic->PreDraw();
		srvManager->PreDraw();

		spriteBasic->SpritePreDraw();
		
		//directionalLight用のCBufferの場所を設定
		directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource->GetGPUVirtualAddress());

		sprite->Draw();
		sprite2->Draw();

		object3DBasic->Object3DPreDraw();

		//gameScene->Draw();

		object3d->Draw();
		object3dTeapot->Draw();
		object3dMultiMesh->Draw();
		object3dMultiMaterial->Draw();
		object3dBunny->Draw();
		object3dSuzanne->Draw();

		particleManager->Draw();
		
		imguiManager->Draw();

		directXBasic->PostDraw();
		
	}

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	//COMの終了処理
	CoUninitialize();

	imguiManager->Finalize();

	//解放処理
	delete debugcamera;
	CloseHandle(directXBasic->GetFenceEvent());
	
	//WindowsApiの終了処理
	winApi->Finalize();

	delete object3dSuzanne;
	delete object3dBunny;
	delete object3dMultiMaterial;
	delete object3dMultiMesh;
	delete object3dTeapot;
	delete object3d;
	delete sprite2;
	delete sprite;
	delete object3DBasic;
	delete spriteBasic;
	delete particleManager;
	gameScene->Finalize();
	delete gameScene;
	//入力解放
	delete input;
	//音声データ解放
	xaudio2Basic->Finalize();
	delete modelManager;
	delete textureManager;
	delete imguiManager;
	delete srvManager;
	delete directXBasic;
	delete winApi;
	delete xaudio2Basic;

	return 0;
}