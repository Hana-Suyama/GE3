#include "Engine.h"
#include "debug/Dump/ExportDump.h"

void Engine::Initialize()
{
	//誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	//loggerの初期化
	logger = std::make_unique<Logger>();
	logger->Initialize();

	//起動時にログ出力のテスト
	logger->Log("test\n");

	//WindowsApi
	//WinApiの初期化
	winApi = std::make_unique<WindowsApi>();
	winApi->Initialize();

	//DirectXの初期化
	directXBasic = std::make_unique<DirectXBasic>();
	directXBasic->Initialize(logger.get(), winApi.get());

	srvManager = std::make_unique<SRVManager>();
	srvManager->Initialize(directXBasic.get());

	imguiManager = std::make_unique<ImGuiManager>();
	imguiManager->Initialize(winApi.get(), directXBasic.get(), srvManager.get());

	//テクスチャマネージャの初期化
	textureManager = std::make_unique<TextureManager>();
	textureManager->Initialize(directXBasic.get(), srvManager.get());

	//モデルマネージャの初期化
	modelManager = std::make_unique<ModelManager>();
	modelManager->Initialize(directXBasic.get(), textureManager.get());

	xaudio2Basic = std::make_unique<XAudio2Basic>();
	xaudio2Basic->Initialize();

	//ポインタ
	//入力の初期化
	input = std::make_unique<Input>();
	input->Initialize(winApi.get());

	logger->Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

	//スプライト基盤
	spriteBasic = std::make_unique<SpriteBasic>();
	spriteBasic->Initialize(directXBasic.get(), logger.get());

	defaultCamera = std::make_unique<Camera>();
	defaultCamera->SetRotate({ 0.0f, 0.0f, 0.0f });
	defaultCamera->SetTranslate({ 0.0f, 0.0f, -10.0f });

	//3Dオブジェクト基盤
	object3DBasic = std::make_unique<Object3DBasic>();
	object3DBasic->Initialize(directXBasic.get(), logger.get());

	object3DBasic->SetDefaultCamera(defaultCamera.get());

	debugcamera = std::make_unique<DebugCamera>();
	debugcamera->Initialize(WindowsApi::kClientWidth, WindowsApi::kClientHeight);

	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(directXBasic.get(), object3DBasic.get(), modelManager.get(), input.get(), logger.get(), srvManager.get(), textureManager.get(), spriteBasic.get(), xaudio2Basic.get(), &randomEngine);
	
}

void Engine::Update()
{

	input->Update();

	sceneManager_->Update();

}

void Engine::PreDraw()
{
	directXBasic->PreDraw();
	srvManager->PreDraw();

}

void Engine::SpritePreDraw()
{
	spriteBasic->SpritePreDraw();

	sceneManager_->SpriteDraw();
}

void Engine::ModelPreDraw()
{
	object3DBasic->Object3DPreDraw();

	sceneManager_->ModelDraw();
}

void Engine::PostDraw()
{
	directXBasic->PostDraw();
}

void Engine::Finalize()
{
	//COMの終了処理
	CoUninitialize();

	imguiManager->Finalize();

	//解放処理
	CloseHandle(directXBasic->GetFenceEvent());

	//WindowsApiの終了処理
	winApi->Finalize();
	
	xaudio2Basic->Finalize();
}

void Engine::Run()
{
	Initialize();

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//Windowsのメッセージ処理
		if (winApi->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		Update();
	}

	Finalize();
}
