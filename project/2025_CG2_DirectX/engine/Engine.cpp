#include "Engine.h"
#include "debug/Dump/ExportDump.h"

void Engine::Initialize()
{
	//誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	//loggerの初期化
	logger = new Logger();
	logger->Initialize();

	//起動時にログ出力のテスト
	logger->Log("test\n");

	//WindowsApi
	//WinApiの初期化
	winApi = new WindowsApi();
	winApi->Initialize();

	//DirectXの初期化
	directXBasic = new DirectXBasic();
	directXBasic->Initialize(logger, winApi);

	srvManager = new SRVManager();
	srvManager->Initialize(directXBasic);

	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApi, directXBasic, srvManager);

	//テクスチャマネージャの初期化
	textureManager = new TextureManager();
	textureManager->Initialize(directXBasic, srvManager);

	//モデルマネージャの初期化
	modelManager = new ModelManager();
	modelManager->Initialize(directXBasic, textureManager);

	xaudio2Basic = new XAudio2Basic();
	xaudio2Basic->Initialize();

	//ポインタ
	//入力の初期化
	input = new Input();
	input->Initialize(winApi);

	logger->Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

	//スプライト基盤
	spriteBasic = new SpriteBasic();
	spriteBasic->Initialize(directXBasic, logger);

	defaultCamera->SetRotate({ 0.0f, 0.0f, 0.0f });
	defaultCamera->SetTranslate({ 0.0f, 0.0f, -10.0f });

	//3Dオブジェクト基盤
	object3DBasic = new Object3DBasic();
	object3DBasic->Initialize(directXBasic, logger);

	object3DBasic->SetDefaultCamera(defaultCamera);

	
	debugcamera->Initialize(WindowsApi::kClientWidth, WindowsApi::kClientHeight);
	
}

void Engine::Update()
{

	input->Update();


	
}

void Engine::PreDraw()
{
	directXBasic->PreDraw();
	srvManager->PreDraw();

}

void Engine::SpritePreDraw()
{
	spriteBasic->SpritePreDraw();
}

void Engine::ModelPreDraw()
{
	object3DBasic->Object3DPreDraw();
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
	delete debugcamera;
	CloseHandle(directXBasic->GetFenceEvent());

	//WindowsApiの終了処理
	winApi->Finalize();

	delete object3DBasic;
	delete spriteBasic;
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
