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
#include "TextureManager.h"
using namespace MyMath;

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xaudio2.lib")

struct DirectionalLight {
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};

struct MaterialData {
	std::string textureFilePath;
};

struct Mesh {
	std::vector<VertexData> vertices;
	MaterialData material;
};

struct ModelData {
	std::vector<Mesh> mesh;
	std::string mtlFileName;
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

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName) {
	MaterialData materialData;	//構築するMaterialData
	std::string line;	//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);	//ファイルを開く
	assert(file.is_open());	//とりあえず開けなかったら止める

	bool readMtl = false;

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//使うマテリアル情報の行まで飛ばす
		if (identifier == "newmtl") {
			std::string useMaterialName;
			s >> useMaterialName;
			if (useMaterialName == materialName) {
				readMtl = true;
			}
		}
		if (!readMtl) {
			continue;
		}

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath+"/"+textureFilename;
			return materialData;
		}
	}
	//マテリアルデータが見つからない
	assert(readMtl);
	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData;	//構築するModelData
	std::vector<Vector4> positions;	//位置
	std::vector<Vector3> normals;	//法線
	std::vector<Vector2> texcoords;	//テクスチャ座標
	std::string line;	//ファイルから読んだ1行を格納するもの
	int32_t meshCount = 0;
	//0番目のメッシュデータを追加
	modelData.mesh.push_back(Mesh{});

	std::ifstream file(directoryPath + "/" + filename);	//ファイルを開く
	assert(file.is_open());	//とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');	// /区切りでインデックスを読んでいく
					if (!texcoords.size() && element == 1) {
						continue;
					}
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord{};
				int32_t falseUV{};
				if (texcoords.size()) {
					texcoord = texcoords[elementIndices[1] - 1];
				} else {
					falseUV = true;
				}
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position, texcoord, normal, falseUV };

			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.mesh[meshCount].vertices.push_back(triangle[2]);
			modelData.mesh[meshCount].vertices.push_back(triangle[1]);
			modelData.mesh[meshCount].vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			//modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
			//マテリアルファイル名を保存しておく
			modelData.mtlFileName = materialFilename;
		} else if (identifier == "o") {
			if (modelData.mesh[meshCount].vertices.size() != 0) {
				meshCount++;
				//空のメッシュデータを追加
				modelData.mesh.push_back(Mesh{});
			}
		} else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.mesh[meshCount].material = LoadMaterialTemplateFile(directoryPath, modelData.mtlFileName, materialName);
		}

	}
	return modelData;

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

	//テクスチャマネージャの初期化
	TextureManager* textureManager = nullptr;
	textureManager = new TextureManager();
	textureManager->Initialize(directXBasic);

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

	////モデル読み込み
	//ModelData modelData = LoadObjFile("resources", "plane.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResources(modelData.mesh.size());
	//for (int32_t i = 0; i < vertexResources.size(); i++) {
	//	vertexResources[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelData.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews(modelData.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViews.size(); i++) {
	//	vertexBufferViews[i].BufferLocation = vertexResources[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViews[i].SizeInBytes = UINT(sizeof(VertexData) * modelData.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViews[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatas(modelData.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatas.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatas[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatas[i], modelData.mesh[i].vertices.data(), sizeof(VertexData) * modelData.mesh[i].vertices.size());
	//}

	//const uint32_t kSubdivision = 16;//分割数
	//const float kLonEvery = float(M_PI) * 2.0f / float(kSubdivision);//経度分割1つ分の角度
	//const float kLatEvery = float(M_PI) / float(kSubdivision);//緯度分割1つ分の角度

	////球を構成する頂点の数
	//int32_t vertexTotalNumber = kSubdivision * kSubdivision * 6;

	////球の頂点リソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = directXBasic->CreateBufferResource(sizeof(VertexData) * vertexTotalNumber);
	////球用の頂点バッファビュー
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	////リソースの先頭のアドレスから使う
	//vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	////使用するリソースのサイズは分割数×分割数×6のサイズ
	//vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * vertexTotalNumber;
	////1頂点当たりのサイズ
	//vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
	//////球の頂点リソースにデータを書き込む
	//VertexData* vertexDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));

	//float space = 1.0f / kSubdivision;

	////緯度の方向に分割 0 ~ 2π
	//for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = float(-M_PI) / 2.0f + kLatEvery * latIndex;//現在の緯度
	//	//経度の方向に分割 0 ~ 2π
	//	for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = lonIndex * kLonEvery;//現在の経度

	//		//頂点にデータを入力する。基準点a
	//		//a(左下)
	//		vertexDataSphere[start].position.x = cosf(lat) * cosf(lon);
	//		vertexDataSphere[start].position.y = sinf(lat);
	//		vertexDataSphere[start].position.z = cosf(lat) * sinf(lon);
	//		vertexDataSphere[start].position.w = 1.0f;
	//		vertexDataSphere[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };
	//		vertexDataSphere[start].normal.x = vertexDataSphere[start].position.x;
	//		vertexDataSphere[start].normal.y = vertexDataSphere[start].position.y;
	//		vertexDataSphere[start].normal.z = vertexDataSphere[start].position.z;

	//		//b(左上)
	//		vertexDataSphere[start + 1].position.x = cosf(lat + kLatEvery) * cosf(lon);
	//		vertexDataSphere[start + 1].position.y = sinf(lat + kLatEvery);
	//		vertexDataSphere[start + 1].position.z = cosf(lat + kLatEvery) * sinf(lon);
	//		vertexDataSphere[start + 1].position.w = 1.0f;
	//		vertexDataSphere[start + 1].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) - space };
	//		vertexDataSphere[start + 1].normal.x = vertexDataSphere[start + 1].position.x;
	//		vertexDataSphere[start + 1].normal.y = vertexDataSphere[start + 1].position.y;
	//		vertexDataSphere[start + 1].normal.z = vertexDataSphere[start + 1].position.z;

	//		//c(右下)
	//		vertexDataSphere[start + 2].position.x = cosf(lat) * cosf(lon + kLonEvery);
	//		vertexDataSphere[start + 2].position.y = sinf(lat);
	//		vertexDataSphere[start + 2].position.z = cosf(lat) * sinf(lon + kLonEvery);
	//		vertexDataSphere[start + 2].position.w = 1.0f;
	//		vertexDataSphere[start + 2].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) };
	//		vertexDataSphere[start + 2].normal.x = vertexDataSphere[start + 2].position.x;
	//		vertexDataSphere[start + 2].normal.y = vertexDataSphere[start + 2].position.y;
	//		vertexDataSphere[start + 2].normal.z = vertexDataSphere[start + 2].position.z;

	//		//c(右下)
	//		vertexDataSphere[start + 3].position.x = cosf(lat) * cosf(lon + kLonEvery);
	//		vertexDataSphere[start + 3].position.y = sinf(lat);
	//		vertexDataSphere[start + 3].position.z = cosf(lat) * sinf(lon + kLonEvery);
	//		vertexDataSphere[start + 3].position.w = 1.0f;
	//		vertexDataSphere[start + 3].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) };
	//		vertexDataSphere[start + 3].normal.x = vertexDataSphere[start + 3].position.x;
	//		vertexDataSphere[start + 3].normal.y = vertexDataSphere[start + 3].position.y;
	//		vertexDataSphere[start + 3].normal.z = vertexDataSphere[start + 3].position.z;

	//		//b(左上)
	//		vertexDataSphere[start + 4].position.x = cosf(lat + kLatEvery) * cosf(lon);
	//		vertexDataSphere[start + 4].position.y = sinf(lat + kLatEvery);
	//		vertexDataSphere[start + 4].position.z = cosf(lat + kLatEvery) * sinf(lon);
	//		vertexDataSphere[start + 4].position.w = 1.0f;
	//		vertexDataSphere[start + 4].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) - space };
	//		vertexDataSphere[start + 4].normal.x = vertexDataSphere[start + 4].position.x;
	//		vertexDataSphere[start + 4].normal.y = vertexDataSphere[start + 4].position.y;
	//		vertexDataSphere[start + 4].normal.z = vertexDataSphere[start + 4].position.z;

	//		//d(右上)
	//		vertexDataSphere[start + 5].position.x = cosf(lat + kLatEvery) * cosf(lon + kLonEvery);
	//		vertexDataSphere[start + 5].position.y = sinf(lat + kLatEvery);
	//		vertexDataSphere[start + 5].position.z = cosf(lat + kLatEvery) * sinf(lon + kLonEvery);
	//		vertexDataSphere[start + 5].position.w = 1.0f;
	//		vertexDataSphere[start + 5].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) - space };
	//		vertexDataSphere[start + 5].normal.x = vertexDataSphere[start + 5].position.x;
	//		vertexDataSphere[start + 5].normal.y = vertexDataSphere[start + 5].position.y;
	//		vertexDataSphere[start + 5].normal.z = vertexDataSphere[start + 5].position.z;

	//	}
	//}

	////UtahTeapotモデル読み込み
	//ModelData modelDataTeapot = LoadObjFile("resources", "teapot.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < vertexResourcesTeapot.size(); i++) {
	//	vertexResourcesTeapot[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsTeapot(modelDataTeapot.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViewsTeapot.size(); i++) {
	//	vertexBufferViewsTeapot[i].BufferLocation = vertexResourcesTeapot[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViewsTeapot[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViewsTeapot[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatasTeapot.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasTeapot[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatasTeapot[i], modelDataTeapot.mesh[i].vertices.data(), sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());
	//}

	////Stanford Bunnyモデル読み込み
	//ModelData modelDataBunny = LoadObjFile("resources", "bunny.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < vertexResourcesBunny.size(); i++) {
	//	vertexResourcesBunny[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsBunny(modelDataBunny.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViewsBunny.size(); i++) {
	//	vertexBufferViewsBunny[i].BufferLocation = vertexResourcesBunny[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViewsBunny[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViewsBunny[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatasBunny(modelDataBunny.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatasBunny.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasBunny[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatasBunny[i], modelDataBunny.mesh[i].vertices.data(), sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());
	//}

	////Multi Meshモデル読み込み
	//ModelData modelDataMultiMesh = LoadObjFile("resources", "multiMesh.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < vertexResourcesMultiMesh.size(); i++) {
	//	vertexResourcesMultiMesh[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViewsMultiMesh.size(); i++) {
	//	vertexBufferViewsMultiMesh[i].BufferLocation = vertexResourcesMultiMesh[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViewsMultiMesh[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViewsMultiMesh[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatasMultiMesh.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasMultiMesh[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatasMultiMesh[i], modelDataMultiMesh.mesh[i].vertices.data(), sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());
	//}

	////Multi Materialモデル読み込み
	//ModelData modelDataMultiMaterial = LoadObjFile("resources", "multiMaterial.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < vertexResourcesMultiMaterial.size(); i++) {
	//	vertexResourcesMultiMaterial[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViewsMultiMaterial.size(); i++) {
	//	vertexBufferViewsMultiMaterial[i].BufferLocation = vertexResourcesMultiMaterial[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViewsMultiMaterial[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViewsMultiMaterial[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatasMultiMaterial.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasMultiMaterial[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatasMultiMaterial[i], modelDataMultiMaterial.mesh[i].vertices.data(), sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());
	//}

	////Suzanne モデル読み込み
	//ModelData modelDataSuzanne = LoadObjFile("resources", "suzanne.obj");
	////メッシュの分だけ頂点リソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesSuzanne(modelDataSuzanne.mesh.size());
	//for (int32_t i = 0; i < vertexResourcesSuzanne.size(); i++) {
	//	vertexResourcesSuzanne[i] = directXBasic->CreateBufferResource(sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());
	//}
	////メッシュの分だけ頂点バッファビューを作成する
	//std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsSuzanne(modelDataSuzanne.mesh.size(), {});
	//for (int32_t i = 0; i < vertexBufferViewsSuzanne.size(); i++) {
	//	vertexBufferViewsSuzanne[i].BufferLocation = vertexResourcesSuzanne[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
	//	vertexBufferViewsSuzanne[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//	vertexBufferViewsSuzanne[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	//}
	//////頂点リソースにデータを書き込む
	//std::vector<VertexData*> vertexDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	//for (int32_t i = 0; i < vertexDatasSuzanne.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	vertexResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasSuzanne[i]));
	//	//頂点データをリソースにコピー
	//	std::memcpy(vertexDatasSuzanne[i], modelDataSuzanne.mesh[i].vertices.data(), sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());
	//}

	//// Meshの分だけマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResources(modelData.mesh.size());
	//for (int32_t i = 0; i < materialResources.size(); i++) {
	//	materialResources[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatas(modelData.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatas.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatas[i]));
	//	//今回は白を書き込んでみる
	//	materialDatas[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatas[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatas[i]->uvTransform = MakeIdentity4x4();
	//}

	////球用のマテリアルリソースを作る
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = directXBasic->CreateBufferResource(sizeof(Material));
	////Sprite用のマテリアルにデータを書き込む
	//Material* materialDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	////白
	//materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	////SpriteはLightingしないのでfalseを設定する
	//materialDataSphere->enableLighting = Light::HalfLambert;
	////UVTransformを単位行列で初期化
	//materialDataSphere->uvTransform = MakeIdentity4x4();

	//// Meshの分だけUtahTeapotマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < materialResourcesTeapot.size(); i++) {
	//	materialResourcesTeapot[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatasTeapot.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasTeapot[i]));
	//	//今回は白を書き込んでみる
	//	materialDatasTeapot[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatasTeapot[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatasTeapot[i]->uvTransform = MakeIdentity4x4();
	//}

	//// Meshの分だけStanford Bunnyマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < materialResourcesBunny.size(); i++) {
	//	materialResourcesBunny[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatasBunny(modelDataBunny.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatasBunny.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasBunny[i]));
	//	//今回は白を書き込んでみる
	//	materialDatasBunny[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatasBunny[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatasBunny[i]->uvTransform = MakeIdentity4x4();
	//}

	//// Meshの分だけMulti Meshマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < materialResourcesMultiMesh.size(); i++) {
	//	materialResourcesMultiMesh[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatasMultiMesh.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasMultiMesh[i]));
	//	//今回は白を書き込んでみる
	//	materialDatasMultiMesh[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatasMultiMesh[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatasMultiMesh[i]->uvTransform = MakeIdentity4x4();
	//}

	//// Meshの分だけMulti Materialマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < materialResourcesMultiMaterial.size(); i++) {
	//	materialResourcesMultiMaterial[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatasMultiMaterial.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasMultiMaterial[i]));
	//	//今回は白を書き込んでみる
	//	materialDatasMultiMaterial[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatasMultiMaterial[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatasMultiMaterial[i]->uvTransform = MakeIdentity4x4();
	//}

	//// Meshの分だけSuzanneマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesSuzanne(modelDataSuzanne.mesh.size());
	//for (int32_t i = 0; i < materialResourcesSuzanne.size(); i++) {
	//	materialResourcesSuzanne[i] = directXBasic->CreateBufferResource(sizeof(Material));
	//}
	////マテリアルにデータを書き込む
	//std::vector<Material*> materialDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	//for (int32_t i = 0; i < materialDatasSuzanne.size(); i++) {
	//	//書き込むためのアドレスを取得
	//	materialResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasSuzanne[i]));
	//	//今回は白を書き込んでみる
	//	materialDatasSuzanne[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//	//Lighting有効
	//	materialDatasSuzanne[i]->enableLighting = Light::HalfLambert;
	//	//UVTransformを単位行列で初期化
	//	materialDatasSuzanne[i]->uvTransform = MakeIdentity4x4();
	//}

	//// WVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixData = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	////単位行列を書き込んでおく
	//transformationMatrixData->WVP = MakeIdentity4x4();
	//transformationMatrixData->World = MakeIdentity4x4();

	//// 球用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSphere = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSphere));
	////単位行列を書き込んでおく
	//transformationMatrixDataSphere->WVP = MakeIdentity4x4();
	//transformationMatrixDataSphere->World = MakeIdentity4x4();

	//// UtahTeapot用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceTeapot = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataTeapot = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceTeapot->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataTeapot));
	////単位行列を書き込んでおく
	//transformationMatrixDataTeapot->WVP = MakeIdentity4x4();
	//transformationMatrixDataTeapot->World = MakeIdentity4x4();

	//// Stanford Bunny用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceBunny = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataBunny = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceBunny->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataBunny));
	////単位行列を書き込んでおく
	//transformationMatrixDataBunny->WVP = MakeIdentity4x4();
	//transformationMatrixDataBunny->World = MakeIdentity4x4();

	//// Multi Mesh用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceMultiMesh = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataMultiMesh = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceMultiMesh->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataMultiMesh));
	////単位行列を書き込んでおく
	//transformationMatrixDataMultiMesh->WVP = MakeIdentity4x4();
	//transformationMatrixDataMultiMesh->World = MakeIdentity4x4();

	//// Multi Material用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceMultiMaterial = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataMultiMaterial = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceMultiMaterial->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataMultiMaterial));
	////単位行列を書き込んでおく
	//transformationMatrixDataMultiMaterial->WVP = MakeIdentity4x4();
	//transformationMatrixDataMultiMaterial->World = MakeIdentity4x4();

	//// Suzanne用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSuzanne = directXBasic->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* transformationMatrixDataSuzanne = nullptr;
	////書き込むためのアドレスを取得
	//transformationMatrixResourceSuzanne->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSuzanne));
	////単位行列を書き込んでおく
	//transformationMatrixDataSuzanne->WVP = MakeIdentity4x4();
	//transformationMatrixDataSuzanne->World = MakeIdentity4x4();

	////plane用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResources(modelData.mesh.size());
	//for (int32_t i = 0; i < indexResources.size(); i++) {
	//	indexResources[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelData.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViews(modelData.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViews.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViews[i].BufferLocation = indexResources[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViews[i].SizeInBytes = UINT(sizeof(uint32_t) * modelData.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////plane用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatas(modelData.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatas.size(); i++) {
	//	indexResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatas[i]));
	//	for (uint32_t j = 0; j < modelData.mesh[i].vertices.size(); j++) {
	//		indexDatas[i][j] = j;
	//	}
	//}

	////球用のindexリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSphere = directXBasic->CreateBufferResource(sizeof(uint32_t) * vertexTotalNumber);
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};
	////リソースの先頭のアドレスから使う
	//indexBufferViewSphere.BufferLocation = indexResourceSphere->GetGPUVirtualAddress();
	////使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//indexBufferViewSphere.SizeInBytes = UINT(sizeof(uint32_t) * vertexTotalNumber);
	////インデックスはuint32_tとする
	//indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;
	////球用インデックスリソースにデータを書き込む
	//uint32_t* indexDataSphere = nullptr;
	//indexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));
	//for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = float(-M_PI) / 2.0f + kLatEvery * latIndex;//現在の緯度
	//	//経度の方向に分割 0 ~ 2π
	//	for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = lonIndex * kLonEvery;//現在の経度
	//		indexDataSphere[start] = start;
	//		indexDataSphere[start + 1] = start + 1;
	//		indexDataSphere[start + 2] = start + 2;
	//		indexDataSphere[start + 3] = start + 1;
	//		indexDataSphere[start + 4] = start + 5;
	//		indexDataSphere[start + 5] = start + 2;
	//	}
	//}

	////UtahTeapot用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < indexResourcesTeapot.size(); i++) {
	//	indexResourcesTeapot[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelDataTeapot.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsTeapot(modelDataTeapot.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViewsTeapot.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewsTeapot[i].BufferLocation = indexResourcesTeapot[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViewsTeapot[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataTeapot.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViewsTeapot[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////UtahTeapot用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatasTeapot.size(); i++) {
	//	indexResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasTeapot[i]));
	//	for (uint32_t j = 0; j < modelDataTeapot.mesh[i].vertices.size(); j++) {
	//		indexDatasTeapot[i][j] = j;
	//	}
	//}

	////Stanford Bunny用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < indexResourcesBunny.size(); i++) {
	//	indexResourcesBunny[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelDataBunny.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsBunny(modelDataBunny.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViewsBunny.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewsBunny[i].BufferLocation = indexResourcesBunny[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViewsBunny[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataBunny.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViewsBunny[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////Stanford Bunny用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatasBunny(modelDataBunny.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatasBunny.size(); i++) {
	//	indexResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasBunny[i]));
	//	for (uint32_t j = 0; j < modelDataBunny.mesh[i].vertices.size(); j++) {
	//		indexDatasBunny[i][j] = j;
	//	}
	//}

	////Multi Mesh用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < indexResourcesMultiMesh.size(); i++) {
	//	indexResourcesMultiMesh[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelDataMultiMesh.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViewsMultiMesh.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewsMultiMesh[i].BufferLocation = indexResourcesMultiMesh[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViewsMultiMesh[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataMultiMesh.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViewsMultiMesh[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////Multi Mesh用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatasMultiMesh.size(); i++) {
	//	indexResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasMultiMesh[i]));
	//	for (uint32_t j = 0; j < modelDataMultiMesh.mesh[i].vertices.size(); j++) {
	//		indexDatasMultiMesh[i][j] = j;
	//	}
	//}

	////Multi Material用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < indexResourcesMultiMaterial.size(); i++) {
	//	indexResourcesMultiMaterial[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelDataMultiMaterial.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViewsMultiMaterial.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewsMultiMaterial[i].BufferLocation = indexResourcesMultiMaterial[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViewsMultiMaterial[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataMultiMaterial.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViewsMultiMaterial[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////Multi Material用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatasMultiMaterial.size(); i++) {
	//	indexResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasMultiMaterial[i]));
	//	for (uint32_t j = 0; j < modelDataMultiMaterial.mesh[i].vertices.size(); j++) {
	//		indexDatasMultiMaterial[i][j] = j;
	//	}
	//}

	////Suzanne用のindexリソース
	////メッシュの分だけindexリソースを作る
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesSuzanne(modelDataSuzanne.mesh.size());
	//for (int32_t i = 0; i < indexResourcesSuzanne.size(); i++) {
	//	indexResourcesSuzanne[i] = directXBasic->CreateBufferResource(sizeof(uint32_t) * modelDataSuzanne.mesh[i].vertices.size());
	//}
	////メッシュの分だけindexバッファビューを作る
	//std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsSuzanne(modelDataSuzanne.mesh.size(), {});
	//for (int32_t i = 0; i < indexBufferViewsSuzanne.size(); i++) {
	//	//リソースの先頭のアドレスから使う
	//	indexBufferViewsSuzanne[i].BufferLocation = indexResourcesSuzanne[i]->GetGPUVirtualAddress();
	//	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	//	indexBufferViewsSuzanne[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataSuzanne.mesh[i].vertices.size());
	//	//インデックスはuint32_tとする
	//	indexBufferViewsSuzanne[i].Format = DXGI_FORMAT_R32_UINT;
	//}
	////Suzanne用インデックスリソースにデータを書き込む
	//std::vector<uint32_t*> indexDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	//for (int32_t i = 0; i < indexDatasSuzanne.size(); i++) {
	//	indexResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasSuzanne[i]));
	//	for (uint32_t j = 0; j < modelDataSuzanne.mesh[i].vertices.size(); j++) {
	//		indexDatasSuzanne[i][j] = j;
	//	}
	//}

	////Transform変数を作る
	//struct Transform transform { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformSphere { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformTeapot { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformBunny { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformMultiMesh { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformMultiMaterial { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	//struct Transform transformSuzanne { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

	////UVTransform変数を作る
	//

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
	textureManager->LoadTexture("resources/checkerBoard.png");

	//sprite
	Sprite* sprite = nullptr;
	sprite = new Sprite();
	sprite->Initialize(spriteBasic, textureManager, "resources/uvChecker.png");

	Sprite* sprite2 = nullptr;
	sprite2 = new Sprite();
	sprite2->Initialize(spriteBasic, textureManager, "resources/monsterBall.png");

	////Textureを読んで転送する
	//DirectX::ScratchImage mipImages = directXBasic->LoadTexture("resources/uvChecker.png");
	//const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = directXBasic->CreateTextureResource(metadata);
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = directXBasic->UploadTextureData(textureResource.Get(), mipImages);

	////モデル用のTextureを読んで転送する
	//std::vector<DirectX::ScratchImage> mipImagesModel(modelData.mesh.size());
	//for (int32_t i = 0; i < mipImagesModel.size(); i++) {
	//	mipImagesModel[i] = directXBasic->LoadTexture(modelData.mesh[i].material.textureFilePath);
	//}
	//const  DirectX::TexMetadata& metadatasModel = mipImagesModel[0].GetMetadata();
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesModel(modelData.mesh.size());
	//for (int32_t i = 0; i < textureResourcesModel.size(); i++) {
	//	textureResourcesModel[i] = directXBasic->CreateTextureResource(metadatasModel);
	//}
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesModel(modelData.mesh.size());
	//for (int32_t i = 0; i < intermediateResourcesModel.size(); i++) {
	//	intermediateResourcesModel[i] = directXBasic->UploadTextureData(textureResourcesModel[i].Get(), mipImagesModel[i]);
	//}

	////Utah TeapotのTextureを読んで転送する
	//std::vector<DirectX::ScratchImage> mipImagesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < mipImagesTeapot.size(); i++) {
	//	mipImagesTeapot[i] = directXBasic->LoadTexture(modelDataTeapot.mesh[i].material.textureFilePath);
	//}
	//const DirectX::TexMetadata& metadatasTeapot = mipImagesTeapot[0].GetMetadata();
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < textureResourcesTeapot.size(); i++) {
	//	textureResourcesTeapot[i] = directXBasic->CreateTextureResource(metadatasTeapot);
	//}
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesTeapot(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < intermediateResourcesTeapot.size(); i++) {
	//	intermediateResourcesTeapot[i] = directXBasic->UploadTextureData(textureResourcesTeapot[i].Get(), mipImagesTeapot[i]);
	//}

	////Stanford BunnyのTextureを読んで転送する
	//std::vector<DirectX::ScratchImage> mipImagesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < mipImagesBunny.size(); i++) {
	//	mipImagesBunny[i] = directXBasic->LoadTexture(modelDataBunny.mesh[i].material.textureFilePath);
	//}
	//const DirectX::TexMetadata& metadatasBunny = mipImagesBunny[0].GetMetadata();
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < textureResourcesBunny.size(); i++) {
	//	textureResourcesBunny[i] = directXBasic->CreateTextureResource(metadatasBunny);
	//}
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesBunny(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < intermediateResourcesBunny.size(); i++) {
	//	intermediateResourcesBunny[i] = directXBasic->UploadTextureData(textureResourcesBunny[i].Get(), mipImagesBunny[i]);
	//}

	////Multi MeshのTextureを読んで転送する
	//std::vector<DirectX::ScratchImage> mipImagesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < mipImagesMultiMesh.size(); i++) {
	//	mipImagesMultiMesh[i] = directXBasic->LoadTexture(modelDataMultiMesh.mesh[i].material.textureFilePath);
	//}
	////const DirectX::TexMetadata& metadatasMultiMesh = mipImagesMultiMesh[0].GetMetadata();
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < textureResourcesMultiMesh.size(); i++) {
	//	textureResourcesMultiMesh[i] = directXBasic->CreateTextureResource(mipImagesMultiMesh[i].GetMetadata());
	//}
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < intermediateResourcesMultiMesh.size(); i++) {
	//	intermediateResourcesMultiMesh[i] = directXBasic->UploadTextureData(textureResourcesMultiMesh[i].Get(), mipImagesMultiMesh[i]);
	//}

	////Multi Material用のTextureを読んで転送する
	//std::vector<DirectX::ScratchImage> mipImagesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < mipImagesMultiMaterial.size(); i++) {
	//	mipImagesMultiMaterial[i] = directXBasic->LoadTexture(modelDataMultiMaterial.mesh[i].material.textureFilePath);
	//}
	////const  DirectX::TexMetadata& metadatasMultiMaterial = mipImagesMultiMaterial[0].GetMetadata();
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < textureResourcesMultiMaterial.size(); i++) {
	//	textureResourcesMultiMaterial[i] = directXBasic->CreateTextureResource(mipImagesMultiMaterial[i].GetMetadata());
	//}
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < intermediateResourcesMultiMaterial.size(); i++) {
	//	intermediateResourcesMultiMaterial[i] = directXBasic->UploadTextureData(textureResourcesMultiMaterial[i].Get(), mipImagesMultiMaterial[i]);
	//}

	////metaDataを基にSRVの設定2
	//std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsModel(modelData.mesh.size(), {});
	//for (int32_t i = 0; i < srvDescsModel.size(); i++) {
	//	srvDescsModel[i].Format = metadatasModel.format;
	//	srvDescsModel[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDescsModel[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDescsModel[i].Texture2D.MipLevels = UINT(metadatasModel.mipLevels);
	//}
	////SRVを作成するDescriptorHeapの位置を決める2
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesModelCPU(modelData.mesh.size());
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesModelGPU(modelData.mesh.size());
	//for (int32_t i = 0; i < modelData.mesh.size(); i++) {
	//	SRVCount++;
	//	textureSrvHandlesModelCPU[i] = directXBasic->GetCPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//	textureSrvHandlesModelGPU[i] = directXBasic->GetGPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//}
	////SRVの生成2
	//for (int32_t i = 0; i < modelData.mesh.size(); i++) {
	//	directXBasic->GetDevice()->CreateShaderResourceView(textureResourcesModel[i].Get(), &srvDescsModel[i], textureSrvHandlesModelCPU[i]);
	//}

	////metaDataを基にSRVの設定Teapot
	//std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsTeapot(modelDataTeapot.mesh.size(), {});
	//for (int32_t i = 0; i < srvDescsTeapot.size(); i++) {
	//	srvDescsTeapot[i].Format = metadatasTeapot.format;
	//	srvDescsTeapot[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDescsTeapot[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDescsTeapot[i].Texture2D.MipLevels = UINT(metadatasTeapot.mipLevels);
	//}
	////SRVを作成するDescriptorHeapの位置を決めるTeapot
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesTeapotCPU(modelDataTeapot.mesh.size());
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesTeapotGPU(modelDataTeapot.mesh.size());
	//for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
	//	SRVCount++;
	//	textureSrvHandlesTeapotCPU[i] = directXBasic->GetCPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//	textureSrvHandlesTeapotGPU[i] = directXBasic->GetGPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//}
	////SRVの生成Teapot
	//for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
	//	directXBasic->GetDevice()->CreateShaderResourceView(textureResourcesTeapot[i].Get(), &srvDescsTeapot[i], textureSrvHandlesTeapotCPU[i]);
	//}

	////metaDataを基にSRVの設定Stanford Bunny
	//std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsBunny(modelDataBunny.mesh.size(), {});
	//for (int32_t i = 0; i < srvDescsBunny.size(); i++) {
	//	srvDescsBunny[i].Format = metadatasBunny.format;
	//	srvDescsBunny[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDescsBunny[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDescsBunny[i].Texture2D.MipLevels = UINT(metadatasBunny.mipLevels);
	//}
	////SRVを作成するDescriptorHeapの位置を決めるStanford Bunny
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesBunnyCPU(modelDataBunny.mesh.size());
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesBunnyGPU(modelDataBunny.mesh.size());
	//for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
	//	SRVCount++;
	//	textureSrvHandlesBunnyCPU[i] = directXBasic->GetCPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//	textureSrvHandlesBunnyGPU[i] = directXBasic->GetGPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//}
	////SRVの生成Stanford Bunny
	//for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
	//	directXBasic->GetDevice()->CreateShaderResourceView(textureResourcesBunny[i].Get(), &srvDescsBunny[i], textureSrvHandlesBunnyCPU[i]);
	//}

	////metaDataを基にSRVの設定Multi Mesh
	//std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	//for (int32_t i = 0; i < srvDescsMultiMesh.size(); i++) {
	//	srvDescsMultiMesh[i].Format = mipImagesMultiMesh[i].GetMetadata().format;
	//	srvDescsMultiMesh[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDescsMultiMesh[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDescsMultiMesh[i].Texture2D.MipLevels = UINT(mipImagesMultiMesh[i].GetMetadata().mipLevels);
	//}
	////SRVを作成するDescriptorHeapの位置を決めるMulti Mesh
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMeshCPU(modelDataMultiMesh.mesh.size());
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMeshGPU(modelDataMultiMesh.mesh.size());
	//for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
	//	SRVCount++;
	//	textureSrvHandlesMultiMeshCPU[i] = directXBasic->GetCPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//	textureSrvHandlesMultiMeshGPU[i] = directXBasic->GetGPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//}
	////SRVの生成Multi Mesh
	//for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
	//	directXBasic->GetDevice()->CreateShaderResourceView(textureResourcesMultiMesh[i].Get(), &srvDescsMultiMesh[i], textureSrvHandlesMultiMeshCPU[i]);
	//}

	////metaDataを基にSRVの設定Multi Material
	//std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	//for (int32_t i = 0; i < srvDescsMultiMaterial.size(); i++) {
	//	srvDescsMultiMaterial[i].Format = mipImagesMultiMaterial[i].GetMetadata().format;
	//	srvDescsMultiMaterial[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDescsMultiMaterial[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//	srvDescsMultiMaterial[i].Texture2D.MipLevels = UINT(mipImagesMultiMaterial[i].GetMetadata().mipLevels);
	//}
	////SRVを作成するDescriptorHeapの位置を決めるMulti Material
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMaterialCPU(modelDataMultiMaterial.mesh.size());
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMaterialGPU(modelDataMultiMaterial.mesh.size());
	//for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
	//	SRVCount++;
	//	textureSrvHandlesMultiMaterialCPU[i] = directXBasic->GetCPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//	textureSrvHandlesMultiMaterialGPU[i] = directXBasic->GetGPUDescriptorHandle(directXBasic->GetSRVDescHeap().Get(), directXBasic->GetSRVHeapSize(), SRVCount);
	//}
	////SRVの生成Multi Material
	//for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
	//	directXBasic->GetDevice()->CreateShaderResourceView(textureResourcesMultiMaterial[i].Get(), &srvDescsMultiMaterial[i], textureSrvHandlesMultiMaterialCPU[i]);
	//}

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

	bool drawSprite = false;
	bool drawPlane = true;
	bool drawSphere = false;
	bool drawTeapot = false;
	bool drawBunny = false;
	bool drawMultiMesh = false;
	bool drawMultiMaterial = false;
	bool drawSuzanne = false;

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

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		
		//Windowsのメッセージ処理
		if (winApi->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		 
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

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

		sprite->Update();
		sprite2->Update();
		//transform.rotate.x += static_cast<float>((input->GetPadKey().lRx - static_cast<LONG>(32767.0)) / 100000.0f);
		if (input->TriggerKeyDown(DIK_SPACE)) {
			drawPlane = !drawPlane;
		}

		//transform.rotate.x += input->GetPadKey().lRx;
		//if (input->TriggerKeyDown(DIK_SPACE)) {
		//	drawPlane = !drawPlane;
		//}

		//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		//Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		//Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		//Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WindowsApi::kClientWidth) / float(WindowsApi::kClientHeight), 0.1f, 100.0f);
		//Matrix4x4 worldViewProjectionMatrix;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixData->WVP = worldViewProjectionMatrix;
		//transformationMatrixData->World = worldMatrix;

		////球の処理
		//Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
		//Matrix4x4 worldViewProjectionMatrixSphere;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataSphere->WVP = worldViewProjectionMatrixSphere;
		//transformationMatrixDataSphere->World = worldMatrixSphere;

		////Utah Teapotの処理
		//Matrix4x4 worldMatrixTeapot = MakeAffineMatrix(transformTeapot.scale, transformTeapot.rotate, transformTeapot.translate);
		//Matrix4x4 worldViewProjectionMatrixTeapot;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixTeapot = Multiply(worldMatrixTeapot, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixTeapot = Multiply(worldMatrixTeapot, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataTeapot->WVP = worldViewProjectionMatrixTeapot;
		//transformationMatrixDataTeapot->World = worldMatrixTeapot;

		////Stanford Bunnyの処理
		//Matrix4x4 worldMatrixBunny = MakeAffineMatrix(transformBunny.scale, transformBunny.rotate, transformBunny.translate);
		//Matrix4x4 worldViewProjectionMatrixBunny;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixBunny = Multiply(worldMatrixBunny, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixBunny = Multiply(worldMatrixBunny, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataBunny->WVP = worldViewProjectionMatrixBunny;
		//transformationMatrixDataBunny->World = worldMatrixBunny;

		////Multi Meshの処理
		//Matrix4x4 worldMatrixMultiMesh = MakeAffineMatrix(transformMultiMesh.scale, transformMultiMesh.rotate, transformMultiMesh.translate);
		//Matrix4x4 worldViewProjectionMatrixMultiMesh;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixMultiMesh = Multiply(worldMatrixMultiMesh, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixMultiMesh = Multiply(worldMatrixMultiMesh, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataMultiMesh->WVP = worldViewProjectionMatrixMultiMesh;
		//transformationMatrixDataMultiMesh->World = worldMatrixMultiMesh;

		////Multi Materialの処理
		//Matrix4x4 worldMatrixMultiMaterial = MakeAffineMatrix(transformMultiMaterial.scale, transformMultiMaterial.rotate, transformMultiMaterial.translate);
		//Matrix4x4 worldViewProjectionMatrixMultiMaterial;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixMultiMaterial = Multiply(worldMatrixMultiMaterial, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixMultiMaterial = Multiply(worldMatrixMultiMaterial, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataMultiMaterial->WVP = worldViewProjectionMatrixMultiMaterial;
		//transformationMatrixDataMultiMaterial->World = worldMatrixMultiMaterial;

		////Suzanneの処理
		//Matrix4x4 worldMatrixSuzanne = MakeAffineMatrix(transformSuzanne.scale, transformSuzanne.rotate, transformSuzanne.translate);
		//Matrix4x4 worldViewProjectionMatrixSuzanne;
		//if (useDebugcamera) {
		//	worldViewProjectionMatrixSuzanne = Multiply(worldMatrixSuzanne, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
		//} else {
		//	worldViewProjectionMatrixSuzanne = Multiply(worldMatrixSuzanne, Multiply(viewMatrix, projectionMatrix));
		//}
		//transformationMatrixDataSuzanne->WVP = worldViewProjectionMatrixSuzanne;
		//transformationMatrixDataSuzanne->World = worldMatrixSuzanne;

		////開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		ImGui::Begin("ImGui");
		if (ImGui::TreeNode("Sprite")) {
			//ImGui::Checkbox("drawSprite", &drawSprite);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&spriteScale), 0, 1000);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&spriteRotation), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&spritePosition), 0, 1000);
			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&spriteColor));
			ImGui::SliderFloat2("Anchor", reinterpret_cast<float*>(&spriteAnchor), -0.5f, 1.5f);
			ImGui::SliderFloat2("LeftTop", reinterpret_cast<float*>(&spriteLeftTop), 0.0f, 1000.0f);
			ImGui::SliderFloat2("RectSize", reinterpret_cast<float*>(&spriteSize), 0.0f, 1000.0f);
			ImGui::Checkbox("FlipX", &spriteFlipX);
			ImGui::Checkbox("FlipY", &spriteFlipY);
			/*ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Sprite2")) {
			//ImGui::Checkbox("drawSprite", &drawSprite);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&spriteScale2), 0, 1000);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&spriteRotation2), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&spritePosition2), 0, 1000);
			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&spriteColor2));
			ImGui::SliderFloat2("Anchor", reinterpret_cast<float*>(&spriteAnchor2), -0.5f, 1.5f);
			ImGui::SliderFloat2("LeftTop", reinterpret_cast<float*>(&spriteLeftTop2), 0.0f, 1000.0f);
			ImGui::SliderFloat2("RectSize", reinterpret_cast<float*>(&spriteSize2), 0.0f, 1000.0f);
			ImGui::Checkbox("FlipX", &spriteFlipX2);
			ImGui::Checkbox("FlipY", &spriteFlipY2);
			/*ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/
			ImGui::TreePop();
		}
		/*if (ImGui::TreeNode("plane.obj")) {
			ImGui::Checkbox("drawPlane", &drawPlane);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transform.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transform.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transform.translate), -5, 5);
			ImGui::Combo("Ligting", &materialDatas[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Sphere")) {
			ImGui::Checkbox("drawSphere", &drawSphere);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSphere.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSphere.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSphere.translate), -5, 5);
			ImGui::Combo("Ligting", &materialDataSphere->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Utah Teapot")) {
			ImGui::Checkbox("drawTeapot", &drawTeapot);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformTeapot.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformTeapot.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformTeapot.translate), -5, 5);
			ImGui::Combo("Ligting", &materialDatasTeapot[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Stanford Bunny")) {
			ImGui::Checkbox("drawTeapot", &drawBunny);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformBunny.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformBunny.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformBunny.translate), -5, 5);
			ImGui::Combo("Ligting", &materialDatasBunny[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Multi Mesh")) {
			ImGui::Checkbox("drawMultiMesh", &drawMultiMesh);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformMultiMesh.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformMultiMesh.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformMultiMesh.translate), -5, 5);
			ImGui::Combo("Ligting 1", &materialDatasMultiMesh[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::Combo("Ligting 2", &materialDatasMultiMesh[1]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Multi Material")) {
			ImGui::Checkbox("drawMultiMaterial", &drawMultiMaterial);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformMultiMaterial.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformMultiMaterial.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformMultiMaterial.translate), -5, 5);
			ImGui::Combo("Ligting 1", &materialDatasMultiMaterial[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::Combo("Ligting 2", &materialDatasMultiMaterial[1]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::DragFloat2("UVTranslate 1", &uvTransformMultiMaterial1.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale 1", &uvTransformMultiMaterial1.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate 1", &uvTransformMultiMaterial1.rotate.z);
			ImGui::DragFloat2("UVTranslate 2", &uvTransformMultiMaterial2.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale 2", &uvTransformMultiMaterial2.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate 2", &uvTransformMultiMaterial2.rotate.z);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Suzanne")) {
			ImGui::Checkbox("drawSuzanne", &drawSuzanne);
			ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSuzanne.scale), -5, 5);
			ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSuzanne.rotate), -5, 5);
			ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSuzanne.translate), -5, 5);
			ImGui::Combo("Ligting", &materialDatasSuzanne[0]->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::TreePop();
		}*/
		if (ImGui::TreeNode("Lighting")) {
			ImGui::SliderFloat3("Direction", reinterpret_cast<float*>(&directionalLightData->direction), -1, 1);
			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&directionalLightData->color));
			ImGui::SliderFloat("Intensity", &directionalLightData->intensity, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		/*if (ImGui::TreeNode("Sound")) {
			if (ImGui::Button("play")) {
				playSound = true;
			}
			ImGui::TreePop();
		}*/
		/*if (ImGui::TreeNode("Key")) {
			ImGui::Text("PushKey : %d", input->PushKey(DIK_SPACE));
			ImGui::Text("TriggerKey : %d", input->TriggerKey(DIK_SPACE));
			ImGui::Text("Gamepad RightJoy : %ld", input->GetPadKey().lRx);
			ImGui::Text("Gamepad RightJoy : %ld", ((input->GetPadKey().lRx - static_cast<LONG>(32767.0)) / static_cast <LONG>(10000.0)));
			ImGui::TreePop();
		}*/
		ImGui::End();

		sprite->SetTranslate(spritePosition);
		sprite->SetRotate(spriteRotation);
		sprite->SetScale(spriteScale);
		sprite->SetColor(spriteColor);
		sprite->SetAnchorPoint(spriteAnchor);
		sprite->SetTextureLeftTop(spriteLeftTop);
		sprite->SetTextureSize(spriteSize);
		sprite->SetIsFlipX(spriteFlipX);
		sprite->SetIsFlipY(spriteFlipY);

		sprite2->SetTranslate(spritePosition2);
		sprite2->SetRotate(spriteRotation2);
		sprite2->SetScale(spriteScale2);
		sprite2->SetColor(spriteColor2);
		sprite2->SetAnchorPoint(spriteAnchor2);
		sprite2->SetTextureLeftTop(spriteLeftTop2);
		sprite2->SetTextureSize(spriteSize2);
		sprite2->SetIsFlipX(spriteFlipX2);
		sprite2->SetIsFlipY(spriteFlipY2);

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
		ImGui::Render();

		directXBasic->PreDraw();

		spriteBasic->SpritePreDraw();
		
		//directionalLight用のCBufferの場所を設定
		directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		sprite->Draw();
		sprite2->Draw();

		//for (int32_t i = 0; i < modelData.mesh.size(); i++) {
		//	//VBVを設定
		//	directXBasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViews[i]);
		//	//wvp用のCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
		//	//マテリアルCBufferの場所を設定
		//	directXBasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResources[i]->GetGPUVirtualAddress());
		//	//IBVを設定
		//	directXBasic->GetCommandList()->IASetIndexBuffer(&indexBufferViews[i]);
		//	//描画！ (DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
		//	if (drawPlane) {
		//		directXBasic->GetCommandList()->DrawIndexedInstanced(UINT(modelData.mesh[i].vertices.size()), 1, 0, 0, 0);
		//	}
		//}

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
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directXBasic->GetCommandList());

		directXBasic->PostDraw();
		
	}

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	//COMの終了処理
	CoUninitialize();

	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//解放処理
	delete debugcamera;
	CloseHandle(directXBasic->GetFenceEvent());
	
	//WindowsApiの終了処理
	winApi->Finalize();

	delete sprite2;
	delete sprite;
	delete spriteBasic;
	//入力解放
	delete input;
	//XAudio2解放
	xAudio2.Reset();
	//音声データ解放
	SoundUnload(&soundData1);
	delete textureManager;
	delete directXBasic;
	delete winApi;

	return 0;
}