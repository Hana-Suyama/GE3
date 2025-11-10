#define _USE_MATH_DEFINES
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
#include <xaudio2.h>
#include "2025_CG2_DirectX/engine/Input.h"
#include "2025_CG2_DirectX/engine/utility/Math/MyMath.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
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
#include "2025_CG2_DirectX/engine/Object3DBasic.h"
#include "2025_CG2_DirectX/engine/Model/ModelManager.h"
#include "2025_CG2_DirectX/engine/Object3D.h"
#include "2025_CG2_DirectX/engine/SRVManager.h"
#include "2025_CG2_DirectX/engine/Particle/ParticleManager.h"
using namespace MyMath;

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xaudio2.lib")

struct DirectionalLight {
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};

//チャンクヘッダ
struct ChunkHeader {
	char id[4];	//チャンク毎のID
	int32_t size;	//チャンクのサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;	//"RIFF"
	char type[4];	//"WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;	//"fmt"
	WAVEFORMATEX fmt;	//波形フォーマット
};

//音声データ
struct SoundData {
	WAVEFORMATEX wfex;	//波形フォーマット
	BYTE* pBuffer;	//バッファの先頭アドレス
	unsigned int bufferSize;	//バッファのサイズ
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

SoundData SoundLoadWave(const char* filename) {
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	assert(file.is_open());

	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//Formatチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//Junkチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	//Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Waveファイルを閉じる
	file.close();

	//returnする為の音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;

}

//音声データ解放
void SoundUnload(SoundData* soundData) {
	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {
	HRESULT result;

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

//Windowsアプリでのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;

	//音声関連
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

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

	//テクスチャマネージャの初期化
	TextureManager* textureManager = nullptr;
	textureManager = new TextureManager();
	textureManager->Initialize(directXBasic, srvManager);

	//モデルマネージャの初期化
	ModelManager* modelManager = nullptr;
	modelManager = new ModelManager();
	modelManager->Initialize(directXBasic, textureManager);

	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));

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

	ParticleManager* particleManager = nullptr;
	particleManager = new ParticleManager();
	particleManager->Initialize(directXBasic, srvManager, logger, textureManager, "resources/uvChecker.png", camera);

	//struct Transform uvTransformMultiMaterial1 {
	//	{ 1.0f, 1.0f, 1.0f },
	//	{ 0.0f, 0.0f, 0.0f },
	//	{ 0.0f, 0.0f, 0.0f },
	//};

	//struct Transform uvTransformMultiMaterial2 {
	//	{ 1.0f, 1.0f, 1.0f },
	//	{ 0.0f, 0.0f, 0.0f },
	//	{ 0.0f, 0.0f, 0.0f },
	//};

	//カメラ用変数を作る
	struct Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -10.0f } };

	textureManager->LoadTexture("resources/uvChecker.png");
	textureManager->LoadTexture("resources/monsterBall.png");

	modelManager->LoadModel("resources", "plane.obj");
	modelManager->LoadModel("resources", "teapot.obj");

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

	//音声読み込み
	SoundData soundData1 = SoundLoadWave("resources/Alarm01.wav");

	BYTE beforeKey[256] = {};
	
	DebugCamera* debugcamera = new DebugCamera();
	debugcamera->Initialize(WindowsApi::kClientWidth, WindowsApi::kClientHeight);
	bool useDebugcamera = false;

	bool playSound = false;

	Vector3 spritePosition{};
	Vector3 spriteRotation{};
	Vector3 spriteScale = sprite->GetTransform().scale;
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 spriteAnchor{};
	Vector2 spriteLeftTop{};
	Vector2 spriteSize = { spriteScale.x, spriteScale.y };
	bool spriteFlipX = false;
	bool spriteFlipY = false;

	Vector3 spritePosition2{};
	Vector3 spriteRotation2{};
	Vector3 spriteScale2 = sprite2->GetTransform().scale;
	Vector4 spriteColor2 = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 spriteAnchor2{};
	Vector2 spriteLeftTop2{};
	Vector2 spriteSize2 = { spriteScale2.x, spriteScale2.y };
	bool spriteFlipX2 = false;
	bool spriteFlipY2 = false;

	Vector3 planePosition{};
	Vector3 planeRotation{};
	Vector3 planeScale = object3d->GetTransform().scale;

	Vector3 teapotPosition{};
	Vector3 teapotRotation{};
	Vector3 teapotScale = object3dTeapot->GetTransform().scale;

	Vector3 EmitterPosition{};

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		
		//Windowsのメッセージ処理
		if (winApi->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		 
		/*ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();*/

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

		camera->Update();

		particleManager->Update(EmitterPosition);

		sprite->Update();
		sprite2->Update();

		object3d->Update(cameraTransform);
		object3dTeapot->Update(cameraTransform);

		////開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		//ImGui::Begin("ImGui");
		//if (ImGui::TreeNode("Sprite")) {
		//	//ImGui::Checkbox("drawSprite", &drawSprite);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&spriteScale), 0, 1000);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&spriteRotation), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&spritePosition), 0, 1000);
		//	ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&spriteColor));
		//	ImGui::SliderFloat2("Anchor", reinterpret_cast<float*>(&spriteAnchor), -0.5f, 1.5f);
		//	ImGui::SliderFloat2("LeftTop", reinterpret_cast<float*>(&spriteLeftTop), 0.0f, 1000.0f);
		//	ImGui::SliderFloat2("RectSize", reinterpret_cast<float*>(&spriteSize), 0.0f, 1000.0f);
		//	ImGui::Checkbox("FlipX", &spriteFlipX);
		//	ImGui::Checkbox("FlipY", &spriteFlipY);
		//	/*ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Sprite2")) {
		//	//ImGui::Checkbox("drawSprite", &drawSprite);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&spriteScale2), 0, 1000);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&spriteRotation2), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&spritePosition2), 0, 1000);
		//	ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&spriteColor2));
		//	ImGui::SliderFloat2("Anchor", reinterpret_cast<float*>(&spriteAnchor2), -0.5f, 1.5f);
		//	ImGui::SliderFloat2("LeftTop", reinterpret_cast<float*>(&spriteLeftTop2), 0.0f, 1000.0f);
		//	ImGui::SliderFloat2("RectSize", reinterpret_cast<float*>(&spriteSize2), 0.0f, 1000.0f);
		//	ImGui::Checkbox("FlipX", &spriteFlipX2);
		//	ImGui::Checkbox("FlipY", &spriteFlipY2);
		//	/*ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("plane.obj")) {
		//	//ImGui::Checkbox("drawPlane", &drawPlane);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&planeScale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&planeRotation), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&planePosition), -5, 5);
		//	if(ImGui::Button("SetPlane")) {
		//		object3d->SetModelData("resources/plane.obj");
		//	}
		//	if (ImGui::Button("SetTeapot")) {
		//		object3d->SetModelData("resources/teapot.obj");
		//	}
		//	//ImGui::Combo("Ligting", &materialDatas[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}
		///*if (ImGui::TreeNode("Sphere")) {
		//	ImGui::Checkbox("drawSphere", &drawSphere);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSphere.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSphere.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSphere.translate), -5, 5);
		//	ImGui::Combo("Ligting", &materialDataSphere->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}*/
		//if (ImGui::TreeNode("Utah Teapot")) {
		//	//ImGui::Checkbox("drawTeapot", &drawTeapot);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&teapotScale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&teapotRotation), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&teapotPosition), -5, 5);
		//	if (ImGui::Button("SetPlane")) {
		//		object3dTeapot->SetModelData("resources/plane.obj");
		//	}
		//	if (ImGui::Button("SetTeapot")) {
		//		object3dTeapot->SetModelData("resources/teapot.obj");
		//	}
		//	//ImGui::Combo("Ligting", &materialDatasTeapot[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Camera")) {
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&cameraTransform.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&cameraTransform.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&cameraTransform.translate), -10, 10);
		//	ImGui::TreePop();
		//}
		///*if (ImGui::TreeNode("Stanford Bunny")) {
		//	ImGui::Checkbox("drawTeapot", &drawBunny);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformBunny.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformBunny.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformBunny.translate), -5, 5);
		//	ImGui::Combo("Ligting", &materialDatasBunny[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Multi Mesh")) {
		//	ImGui::Checkbox("drawMultiMesh", &drawMultiMesh);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformMultiMesh.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformMultiMesh.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformMultiMesh.translate), -5, 5);
		//	ImGui::Combo("Ligting 1", &materialDatasMultiMesh[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::Combo("Ligting 2", &materialDatasMultiMesh[1]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Multi Material")) {
		//	ImGui::Checkbox("drawMultiMaterial", &drawMultiMaterial);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformMultiMaterial.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformMultiMaterial.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformMultiMaterial.translate), -5, 5);
		//	ImGui::Combo("Ligting 1", &materialDatasMultiMaterial[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::Combo("Ligting 2", &materialDatasMultiMaterial[1]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::DragFloat2("UVTranslate 1", &uvTransformMultiMaterial1.translate.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::DragFloat2("UVScale 1", &uvTransformMultiMaterial1.scale.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::SliderAngle("UVRotate 1", &uvTransformMultiMaterial1.rotate.z);
		//	ImGui::DragFloat2("UVTranslate 2", &uvTransformMultiMaterial2.translate.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::DragFloat2("UVScale 2", &uvTransformMultiMaterial2.scale.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::SliderAngle("UVRotate 2", &uvTransformMultiMaterial2.rotate.z);
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Suzanne")) {
		//	ImGui::Checkbox("drawSuzanne", &drawSuzanne);
		//	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSuzanne.scale), -5, 5);
		//	ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSuzanne.rotate), -5, 5);
		//	ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSuzanne.translate), -5, 5);
		//	ImGui::Combo("Ligting", &materialDatasSuzanne[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Lighting")) {
		//	ImGui::SliderFloat3("Direction", reinterpret_cast<float*>(&directionalLightData->direction), -1, 1);
		//	ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&directionalLightData->color));
		//	ImGui::SliderFloat("Intensity", &directionalLightData->intensity, 0.0f, 1.0f);
		//	ImGui::TreePop();
		//}*/
		///*if (ImGui::TreeNode("Sound")) {
		//	if (ImGui::Button("play")) {
		//		playSound = true;
		//	}
		//	ImGui::TreePop();
		//}*/
		///*if (ImGui::TreeNode("Key")) {
		//	ImGui::Text("PushKey : %d", input->PushKey(DIK_SPACE));
		//	ImGui::Text("TriggerKey : %d", input->TriggerKey(DIK_SPACE));
		//	ImGui::Text("Gamepad RightJoy : %ld", input->GetPadKey().lRx);
		//	ImGui::Text("Gamepad RightJoy : %ld", ((input->GetPadKey().lRx - static_cast<LONG>(32767.0)) / static_cast <LONG>(10000.0)));
		//	ImGui::TreePop();
		//}*/
		//ImGui::End();

		if (input->PushKey(DIK_A)) {
			cameraTransform.translate.x -= 0.05f;
		}
		if (input->PushKey(DIK_D)) {
			cameraTransform.translate.x += 0.05f;
		}
		if (input->PushKey(DIK_W)) {
			cameraTransform.translate.y += 0.05f;
		}
		if (input->PushKey(DIK_S)) {
			cameraTransform.translate.y -= 0.05f;
		}

		if (input->PushKey(DIK_RIGHTARROW)) {
			EmitterPosition.x += 0.05f;
		}
		if (input->PushKey(DIK_LEFTARROW)) {
			EmitterPosition.x -= 0.05f;
		}
		if (input->PushKey(DIK_UPARROW)) {
			EmitterPosition.y += 0.05f;
		}
		if (input->PushKey(DIK_DOWNARROW)) {
			EmitterPosition.y -= 0.05f;
		}

		camera->SetTranslate(cameraTransform.translate);
		camera->SetRotate(cameraTransform.rotate);

		spritePosition.x += 3.0f;
		sprite->SetTranslate(spritePosition);
		sprite->SetRotate(spriteRotation);
		sprite->SetScale(spriteScale);
		sprite->SetColor(spriteColor);
		sprite->SetAnchorPoint(spriteAnchor);
		sprite->SetTextureLeftTop(spriteLeftTop);
		sprite->SetTextureSize(spriteSize);
		sprite->SetIsFlipX(spriteFlipX);
		sprite->SetIsFlipY(spriteFlipY);

		spritePosition2.y += 3.0f;
		sprite2->SetTranslate(spritePosition2);
		sprite2->SetRotate(spriteRotation2);
		sprite2->SetScale(spriteScale2);
		sprite2->SetColor(spriteColor2);
		sprite2->SetAnchorPoint(spriteAnchor2);
		sprite2->SetTextureLeftTop(spriteLeftTop2);
		sprite2->SetTextureSize(spriteSize2);
		sprite2->SetIsFlipX(spriteFlipX2);
		sprite2->SetIsFlipY(spriteFlipY2);

		planeRotation.x += 0.01f;
		object3d->SetTranslate(planePosition);
		object3d->SetRotate(planeRotation);
		object3d->SetScale(planeScale);

		object3dTeapot->SetTranslate(teapotPosition);
		object3dTeapot->SetRotate(teapotRotation);
		object3dTeapot->SetScale(teapotScale);

		if (playSound) {
			//音声再生
			SoundPlayWave(xAudio2.Get(), soundData1);
			playSound = false;
		}

		directionalLightData->direction = Normalize(directionalLightData->direction);

		

		/*Matrix4x4 uvTransformMatrixMultiMaterial1 = MakeScaleMatrix(uvTransformMultiMaterial1.scale);
		uvTransformMatrixMultiMaterial1 = Multiply(uvTransformMatrixMultiMaterial1, MakeRotateZMatrix(uvTransformMultiMaterial1.rotate.z));
		uvTransformMatrixMultiMaterial1 = Multiply(uvTransformMatrixMultiMaterial1, MakeTranslateMatrix(uvTransformMultiMaterial1.translate));
		materialDatasMultiMaterial[0]->uvTransform = uvTransformMatrixMultiMaterial1;

		Matrix4x4 uvTransformMatrixMultiMaterial2 = MakeScaleMatrix(uvTransformMultiMaterial2.scale);
		uvTransformMatrixMultiMaterial2 = Multiply(uvTransformMatrixMultiMaterial2, MakeRotateZMatrix(uvTransformMultiMaterial2.rotate.z));
		uvTransformMatrixMultiMaterial2 = Multiply(uvTransformMatrixMultiMaterial2, MakeTranslateMatrix(uvTransformMultiMaterial2.translate));
		materialDatasMultiMaterial[1]->uvTransform = uvTransformMatrixMultiMaterial2;*/

		//ImGuiの内部コマンドを生成する
		//ImGui::Render();

		directXBasic->PreDraw();
		srvManager->PreDraw();

		spriteBasic->SpritePreDraw();
		
		//directionalLight用のCBufferの場所を設定
		directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		/*sprite->Draw();
		sprite2->Draw();*/

		object3DBasic->Object3DPreDraw();

		//object3d->Draw();

		//object3dTeapot->Draw();

		particleManager->Draw();

		////球の描画。変更が必要なものだけ変更する
		//directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);	//VBVを設定
		////TransformationMatrixCBufferの場所を設定
		//directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSphere->GetGPUVirtualAddress());
		////マテリアルCBufferの場所を設定
		//directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
		////IBVを設定
		//directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewSphere);
		//if (drawSphere) {
		//	directXBasic->GetCommandList()->DrawIndexedInstanced(vertexTotalNumber, 1, 0, 0, 0);
		//}

		//for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
		//	//Utah Teapotのテクスチャ
		//	directXBasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandlesTeapotGPU[i]);
		//	//Utah Teapotの描画。変更が必要なものだけ変更する
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewsTeapot[i]);	//VBVを設定
		//	//TransformationMatrixCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceTeapot->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourcesTeapot[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewsTeapot[i]);
		//	if (drawTeapot) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelDataTeapot.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

		//for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
		//	//Stanford Bunnyのテクスチャ
		//	directXBasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandlesBunnyGPU[i]);
		//	//Stanford Bunnyの描画。変更が必要なものだけ変更する
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewsBunny[i]);	//VBVを設定
		//	//TransformationMatrixCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceBunny->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourcesBunny[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewsBunny[i]);
		//	if (drawBunny) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelDataBunny.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

		//for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
		//	//Multi Meshのテクスチャ
		//	directXBasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandlesMultiMeshGPU[i]);
		//	//Multi Meshの描画。変更が必要なものだけ変更する
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewsMultiMesh[i]);	//VBVを設定
		//	//TransformationMatrixCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceMultiMesh->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourcesMultiMesh[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewsMultiMesh[i]);
		//	if (drawMultiMesh) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelDataMultiMesh.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

		//for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
		//	//Multi Meshのテクスチャ
		//	directXBasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandlesMultiMaterialGPU[i]);
		//	//Multi Meshの描画。変更が必要なものだけ変更する
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewsMultiMaterial[i]);	//VBVを設定
		//	//TransformationMatrixCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceMultiMaterial->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourcesMultiMaterial[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewsMultiMaterial[i]);
		//	if (drawMultiMaterial) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelDataMultiMaterial.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

		//for (int32_t i = 0; i < modelDataSuzanne.mesh.size(); i++) {
		//	//Suzanneはテクスチャなし
		//	//Suzanneの描画。変更が必要なものだけ変更する
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewsSuzanne[i]);	//VBVを設定
		//	//TransformationMatrixCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSuzanne->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourcesSuzanne[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViewsSuzanne[i]);
		//	if (drawSuzanne) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelDataSuzanne.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

		

		//実際のcommandListのImGuiの描画コマンドを積む
		//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directXBasic->GetCommandList());

		directXBasic->PostDraw();
		
	}

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	//COMの終了処理
	CoUninitialize();

	////ImGuiの終了処理
	//ImGui_ImplDX12_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();

	//解放処理
	delete debugcamera;
	CloseHandle(directXBasic->GetFenceEvent());
	
	//WindowsApiの終了処理
	winApi->Finalize();

	delete object3dTeapot;
	delete object3d;
	delete sprite2;
	delete sprite;
	delete object3DBasic;
	delete spriteBasic;
	delete particleManager;
	//入力解放
	delete input;
	//XAudio2解放
	xAudio2.Reset();
	//音声データ解放
	SoundUnload(&soundData1);
	delete modelManager;
	delete textureManager;
	delete srvManager;
	delete directXBasic;
	delete winApi;

	return 0;
}