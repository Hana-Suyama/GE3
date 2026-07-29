#include "Engine.h"
#include "ExportDump.h"
#include <TimeManager.h>

void Engine::Initialize()
{
	//誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	//loggerの初期化
	logger_ = std::make_unique<Logger>();
	logger_->Initialize();

	//起動時にログ出力のテスト
	logger_->Log("test\n");

	//WindowsApi
	//WinApiの初期化
	winApi_ = std::make_unique<WindowsApi>();
	winApi_->Initialize();

	//DirectXの初期化
	directXBasic_ = std::make_unique<DirectXBasic>();
	directXBasic_->Initialize(logger_.get(), winApi_.get());

	// SRVManagerの初期化
	srvManager_ = std::make_unique<SRVManager>();
	srvManager_->Initialize(directXBasic_.get());

	// ImGuiManagerの初期化
	imguiManager_ = std::make_unique<ImGuiManager>();
	imguiManager_->Initialize(winApi_.get(), directXBasic_.get(), srvManager_.get());

	//テクスチャマネージャの初期化
	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(directXBasic_.get(), srvManager_.get());

	//モデルマネージャの初期化
	modelManager_ = std::make_unique<ModelManager>();
	modelManager_->Initialize(directXBasic_.get(), textureManager_.get(), srvManager_.get());

	xaudio2Basic_ = std::make_unique<XAudio2Basic>();
	xaudio2Basic_->Initialize();

	//ポインタ
	//入力の初期化
	Input::GetInstance()->Initialize(winApi_.get());

	logger_->Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

	//スプライト基盤
	spriteBasic_ = std::make_unique<SpriteBasic>();
	spriteBasic_->Initialize(directXBasic_.get(), logger_.get());

	defaultCamera_ = std::make_unique<Camera>();
	defaultCamera_->SetRotate({ 0.0f, 0.0f, 0.0f });
	defaultCamera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

	// ポストエフェクトレンダラーの初期化
	postEffectRenderer_ = std::make_unique<PostEffectRenderer>();
	postEffectRenderer_->Initialize(directXBasic_.get(), srvManager_.get(), textureManager_.get());

	//3Dオブジェクト基盤
	object3DBasic_ = std::make_unique<Object3DBasic>();
	object3DBasic_->Initialize(directXBasic_.get(), logger_.get());
	object3DBasic_->SetDefaultCamera(defaultCamera_.get());

	// スキニング3Dオブジェクト基盤
	skinnedObject3DBasic_ = std::make_unique<SkinnedObject3DBasic>();
	skinnedObject3DBasic_->Initialize(directXBasic_.get(), logger_.get());
	skinnedObject3DBasic_->SetDefaultCamera(defaultCamera_.get());

	debugcamera_ = std::make_unique<DebugCamera>();
	debugcamera_->Initialize(WindowsApi::kClientWidth, WindowsApi::kClientHeight);

	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(directXBasic_.get(), object3DBasic_.get(), skinnedObject3DBasic_.get(), modelManager_.get(), logger_.get(), srvManager_.get(), textureManager_.get(), spriteBasic_.get(), xaudio2Basic_.get(), &randomEngine_, &postEffectController_);

	TimeManager::GetInstance()->Initialize();

}

void Engine::Update()
{

	TimeManager::GetInstance()->Update();

	Input::GetInstance()->Update(TimeManager::GetInstance()->GetDeltaTime());

	sceneManager_->Update();

}

void Engine::RenderTexturePreDraw()
{
	directXBasic_->PreDrawRenderTexture();
	srvManager_->PreDraw();
}

void Engine::BackBufferPreDraw()
{
	directXBasic_->PreDrawBackBuffer();
}

void Engine::SceneSpriteDraw()
{
	spriteBasic_->SpritePreDraw();

	sceneManager_->SpriteDraw();
}

void Engine::SceneModelDraw()
{
	object3DBasic_->Object3DPreDraw();

	sceneManager_->ModelDraw();

	skinnedObject3DBasic_->SkinnedObject3DPreDraw();

	sceneManager_->SkinnedModelDraw();
}

void Engine::DrawPostEffect()
{
	postEffectRenderer_->Draw(postEffectController_.GetSettings(), *object3DBasic_->GetDefaultCamera(), TimeManager::GetInstance()->GetDeltaTime());
}

void Engine::PostDraw()
{
	directXBasic_->PostDraw();
}

void Engine::PostEffectDebugDraw() {
	postEffectRenderer_->DebugDraw(postEffectController_.GetSettings());
}

void Engine::Finalize()
{
	// 入力の解放
	Input::GetInstance()->ReleaseInstance();

	//COMの終了処理
	CoUninitialize();

	imguiManager_->Finalize();

	//解放処理
	CloseHandle(directXBasic_->GetFenceEvent());

	//WindowsApiの終了処理
	winApi_->Finalize();
	
	xaudio2Basic_->Finalize();
}

void Engine::Run()
{
	Initialize();

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//Windowsのメッセージ処理
		if (winApi_->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		Update();
	}

	Finalize();
}
