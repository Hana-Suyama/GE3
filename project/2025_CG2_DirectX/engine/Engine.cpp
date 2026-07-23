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

	srvManager_ = std::make_unique<SRVManager>();
	srvManager_->Initialize(directXBasic_.get());

	// ここでレンダーテクスチャ用SRVを作る
	renderTextureSrvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforRendertargetTexture(
		renderTextureSrvIndex,
		directXBasic_->GetRenderTextureResource(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	);

	// DepthTexture用SRVを作る
	depthTextureSrvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(
		depthTextureSrvIndex,
		directXBasic_->GetDepthStencilResource()
	);

	imguiManager_ = std::make_unique<ImGuiManager>();
	imguiManager_->Initialize(winApi_.get(), directXBasic_.get(), srvManager_.get());

	//テクスチャマネージャの初期化
	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(directXBasic_.get(), srvManager_.get());

	// Dissolve用ノイズテクスチャを読み込む
	textureManager_->LoadTexture("resources/noise0.png");

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

	outlineMaterialResource_ = directXBasic_->CreateBufferResource(sizeof(PostEffectMaterial));
	outlineMaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&outlineMaterialData_));
	outlineMaterialData_->projectionInverse = defaultCamera_->GetProjectionMatrix().Inverse();

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

void Engine::PreDraw()
{
	//directXBasic_->PreDraw();
	directXBasic_->PreDrawRenderTexture();
	srvManager_->PreDraw();

}

void Engine::BackBufferPreDraw()
{
	directXBasic_->PreDrawBackBuffer();
}

void Engine::SpritePreDraw()
{
	spriteBasic_->SpritePreDraw();

	sceneManager_->SpriteDraw();
}

void Engine::ModelPreDraw()
{
	object3DBasic_->Object3DPreDraw();

	sceneManager_->ModelDraw();

	skinnedObject3DBasic_->SkinnedObject3DPreDraw();

	sceneManager_->SkinnedModelDraw();
}

void Engine::DrawRenderTexture()
{
	const PostEffectSettings& postEffectSettings = postEffectController_.GetSettings();

	Camera* camera = object3DBasic_->GetDefaultCamera();
	outlineMaterialData_->projectionInverse =
		camera->GetProjectionMatrix().Inverse();

	postEffectTime_ += TimeManager::GetInstance()->GetDeltaTime();
	outlineMaterialData_->time = postEffectTime_;

	outlineMaterialData_->threshold = postEffectSettings.threshold;

	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	directXBasic_->GetCommandList()->SetDescriptorHeaps(1, heaps);

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(directXBasic_->GetRootSignatureRenderTexture());
	directXBasic_->GetCommandList()->SetPipelineState(directXBasic_->GetPostEffectPSO(postEffectSettings.type));	//PSOを設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(0, outlineMaterialResource_->GetGPUVirtualAddress());

	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(
		2,
		srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex)
	);

	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(
		5,
		textureManager_->GetSrvHandleGPU("resources/noise0.png")
	);

	// 頂点3つ描画
	directXBasic_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void Engine::PostDraw()
{
	directXBasic_->PostDraw();
}


void Engine::PostEffectImGuiDraw()
{
#ifdef USE_IMGUI
	PostEffectSettings& postEffectSettings = postEffectController_.GetSettings();

	static constexpr const char* effectNames[] = {
		"None",
		"Grayscale",
		"Sepia",
		"Vignette",
		"BoxFilter3x3" ,
		"BoxFilter5x5" ,
		"GaussianBlur",
		"RadialBlur",
		"LuminanceOutline",
		"DepthOutline",
		"Random",
		"Dissolve"
	};

	int type = static_cast<int>(postEffectSettings.type);

	ImGui::Begin("PostEffect");

	if (ImGui::Combo(
		"Effect",
		&type,
		effectNames,
		static_cast<int>(std::size(effectNames))))
	{
		postEffectSettings.type =
			static_cast<PostEffectType>(type);
	}

	if (postEffectSettings.type == PostEffectType::Dissolve) {
		ImGui::SliderFloat(
			"Threshold",
			&postEffectSettings.threshold,
			0.0f,
			1.0f
		);
	}

	ImGui::End();
#endif
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
