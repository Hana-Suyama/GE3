#define _USE_MATH_DEFINES
#include "cmath"
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
#include "2025_CG2_DirectX/engine/utility/MyMath.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#define DIRECTINPUT_VERSION     0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include "2025_CG2_DirectX/engine/Debug/DebugCamera.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	int32_t falseUV;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

enum Light {
	None,
	Lambert,
	HalfLambert,
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

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

//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	
	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
	//CompileするShaderファイルへのパス
	const std::wstring& filePath,
	//Compileに使用するProfile
	const wchar_t* profile,
	//初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	std::ofstream& logStream)
{
	//これからシェーダーをコンパイルする旨をログに出す
	Log(logStream, ConvertString(std::format(L"Begin CompileShader, path:{}\n", filePath, profile)));
	//hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; //UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(), //コンパイル対象のhlslファイル名
		L"-E", L"main", //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile, //ShaderProfileの設定
		L"-Zi", L"-Qembed_debug", //デバッグ用の情報を埋め込む
		L"-Od",		//最適化を外しておく
		L"-Zpr",	//メモリレイアウトは行優先
	};
	//実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,	//読み込んだファイル
		arguments,				//コンパイルオプション
		_countof(arguments),	//コンパイルオプションの数
		includeHandler,			//includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)	//コンパイル結果
	);
	//コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(logStream, shaderError->GetStringPointer());
		//警告・エラーダメゼッタイ
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(logStream, ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	//shaderSource->Release();
	//shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;

}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes) {
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;//リソースのサイズ。今回はVector4を3頂点分
	//バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata) {
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);	//Textureの幅
	resourceDesc.Height = UINT(metadata.height);	//Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);	//mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);	//奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format;	//TextureのFormat
	resourceDesc.SampleDesc.Count = 1;	//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	//Textureの次元数。普段使っているのは2次元

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,	//Heapの設定
		D3D12_HEAP_FLAG_NONE,	//Heapの特殊な設定。特になし。
		&resourceDesc,	//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,	//データ転送される設定
		nullptr,	//Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));	//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList.Get(), texture, intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	//Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESCからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;	//Textureの幅
	resourceDesc.Height = height;	//Textureの高さ
	resourceDesc.MipLevels = 1;	//mipmapの数
	resourceDesc.DepthOrArraySize = 1;	//奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;	//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;	//VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;	//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//フォーマット。Resourceと合わせる

	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,	//Heapの設定
		D3D12_HEAP_FLAG_NONE,	//Heapの特殊な設定。特になし。
		&resourceDesc,	//Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	//深度値を書き込む状態にしておく
		&depthClearValue,	//Clear最適値
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;

}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
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

bool GetKeyIsPress(const BYTE key[256], uint8_t keyNum) {
	if (key[keyNum]) {
		return true;
	}
	return false;
}

bool GetKeyIsNotPress(const BYTE key[256], uint8_t keyNum) {
	if (key[keyNum]) {
		return false;
	}
	return true;
}

bool GetKeyDown(const BYTE key[256], const BYTE beforeKey[256], uint8_t keyNum) {
	if (key[keyNum] && !beforeKey[keyNum]) {
		return true;
	}
	return false;
}

bool GetKeyUp(const BYTE key[256], const BYTE beforeKey[256], uint8_t keyNum) {
	if (!key[keyNum] && beforeKey[keyNum]) {
		return true;
	}
	return false;
}

struct enumdata
{
	LPDIRECTINPUT8 pInput;                // デバイスを作成するためのインターフェイス
	LPDIRECTINPUTDEVICE8* ppPadDevice;    // 使用するデバイスを格納するポインタのポインタ
};

// デバイス発見時に実行される
BOOL CALLBACK DeviceFindCallBack(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef)
{
	enumdata* ed = (enumdata*)pvRef;

	HRESULT hr;

	hr = ed->pInput->CreateDevice(ipddi->guidInstance, ed->ppPadDevice, NULL);
	assert(SUCCEEDED(hr));

	return DIENUM_CONTINUE;
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

	//ログのディレクトリを用意
	std::filesystem::create_directory("logs");

	//現在時刻を取得(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒入らないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間(PCの設定時間)に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	//ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);

	//起動時にログ出力のテスト
	//Log(logStream, "test\n");

	WNDCLASS wc{};
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウィンドウクラス名(なんでも良い)
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc);

	//クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		//利用するクラス名
		L"CG2",					//タイトルバーの文字(何でも良い)
		WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			//表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left,	//ウィンドウ横幅
		wrc.bottom - wrc.top,	//ウィンドウ縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		wc.hInstance,			//インスタンスハンドル
		nullptr);				//オプション

	//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	#ifdef _DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			//デバッグレイヤーを有効化する
			debugController->EnableDebugLayer();
			//さらにGPU側でもチェックを行うようにする
			debugController->SetEnableGPUBasedValidation(TRUE);
		}
	#endif

	//DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
	DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
	DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事
		//ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Log(logStream, ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);

	//XAudioエンジンのインスタンスを生成
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));

	//DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	hr = DirectInput8Create(
		wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//ゲームパッドデバイスの生成
	IDirectInputDevice8* gamepad = nullptr;
	enumdata ed;
	ed.pInput = directInput;
	ed.ppPadDevice = &gamepad;
	//ゲームパッドの列挙
	hr = directInput->EnumDevices(DI8DEVTYPE_GAMEPAD, DeviceFindCallBack, &ed, DIEDFL_ATTACHEDONLY);
	assert(SUCCEEDED(hr));

	if (gamepad) {
		//入力データ形式のセット
		hr = gamepad->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(hr));

		//排他制御レベルのセット
		hr = gamepad->SetCooperativeLevel(
			hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	}
	
	Log(logStream, "Complete create D3D12Device!!!\n");//初期化完了のログを出す

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		//ヤバいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		//解放
		infoQueue->Release();
	}
#endif
	
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドアロケータを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;		//画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;	//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式
	swapChainDesc.SampleDesc.Count = 1;	//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;	//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//モニタにうつしたら、中身を破壊
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//SwapChainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2dテクスチャとして書き込む
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 0);
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1] = GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 1);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	//初期値0でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	//dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;	//0から始まる
	descriptorRange[0].NumDescriptors = 1;	//数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算

	// RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;	//レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;	//レジスタ番号0を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;	//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);	//Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;	//レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters;	//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);	//配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//0~1の範囲外をリビート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのMipMapを使う
	staticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[4] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[3].SemanticName = "FALSEUV";
	inputElementDescs[3].SemanticIndex = 0;
	inputElementDescs[3].Format = DXGI_FORMAT_R32_SINT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"resources/shaders/Object3D.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//モデル読み込み
	ModelData modelData = LoadObjFile("resources", "plane.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResources(modelData.mesh.size());
	for (int32_t i = 0; i < vertexResources.size(); i++) {
		vertexResources[i] = CreateBufferResource(device, sizeof(VertexData) * modelData.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews(modelData.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViews.size(); i++) {
		vertexBufferViews[i].BufferLocation = vertexResources[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViews[i].SizeInBytes = UINT(sizeof(VertexData) * modelData.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViews[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatas(modelData.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatas.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatas[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatas[i], modelData.mesh[i].vertices.data(), sizeof(VertexData) * modelData.mesh[i].vertices.size());
	}

	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 4);
	//スプライト用頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//スプライト用の頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };//左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].normal = { 0.0f, 0.0f, -1.0f };

	const uint32_t kSubdivision = 16;//分割数
	const float kLonEvery = float(M_PI) * 2.0f / float(kSubdivision);//経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / float(kSubdivision);//緯度分割1つ分の角度

	//球を構成する頂点の数
	int32_t vertexTotalNumber = kSubdivision * kSubdivision * 6;

	//球の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = CreateBufferResource(device, sizeof(VertexData) * vertexTotalNumber);
	//球用の頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	//使用するリソースのサイズは分割数×分割数×6のサイズ
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * vertexTotalNumber;
	//1頂点当たりのサイズ
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
	////球の頂点リソースにデータを書き込む
	VertexData* vertexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));

	float space = 1.0f / kSubdivision;

	//緯度の方向に分割 0 ~ 2π
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = float(-M_PI) / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//現在の経度

			//頂点にデータを入力する。基準点a
			//a(左下)
			vertexDataSphere[start].position.x = cosf(lat) * cosf(lon);
			vertexDataSphere[start].position.y = sinf(lat);
			vertexDataSphere[start].position.z = cosf(lat) * sinf(lon);
			vertexDataSphere[start].position.w = 1.0f;
			vertexDataSphere[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };
			vertexDataSphere[start].normal.x = vertexDataSphere[start].position.x;
			vertexDataSphere[start].normal.y = vertexDataSphere[start].position.y;
			vertexDataSphere[start].normal.z = vertexDataSphere[start].position.z;

			//b(左上)
			vertexDataSphere[start + 1].position.x = cosf(lat + kLatEvery) * cosf(lon);
			vertexDataSphere[start + 1].position.y = sinf(lat + kLatEvery);
			vertexDataSphere[start + 1].position.z = cosf(lat + kLatEvery) * sinf(lon);
			vertexDataSphere[start + 1].position.w = 1.0f;
			vertexDataSphere[start + 1].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) - space };
			vertexDataSphere[start + 1].normal.x = vertexDataSphere[start + 1].position.x;
			vertexDataSphere[start + 1].normal.y = vertexDataSphere[start + 1].position.y;
			vertexDataSphere[start + 1].normal.z = vertexDataSphere[start + 1].position.z;

			//c(右下)
			vertexDataSphere[start + 2].position.x = cosf(lat) * cosf(lon + kLonEvery);
			vertexDataSphere[start + 2].position.y = sinf(lat);
			vertexDataSphere[start + 2].position.z = cosf(lat) * sinf(lon + kLonEvery);
			vertexDataSphere[start + 2].position.w = 1.0f;
			vertexDataSphere[start + 2].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) };
			vertexDataSphere[start + 2].normal.x = vertexDataSphere[start + 2].position.x;
			vertexDataSphere[start + 2].normal.y = vertexDataSphere[start + 2].position.y;
			vertexDataSphere[start + 2].normal.z = vertexDataSphere[start + 2].position.z;

			//c(右下)
			vertexDataSphere[start + 3].position.x = cosf(lat) * cosf(lon + kLonEvery);
			vertexDataSphere[start + 3].position.y = sinf(lat);
			vertexDataSphere[start + 3].position.z = cosf(lat) * sinf(lon + kLonEvery);
			vertexDataSphere[start + 3].position.w = 1.0f;
			vertexDataSphere[start + 3].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) };
			vertexDataSphere[start + 3].normal.x = vertexDataSphere[start + 3].position.x;
			vertexDataSphere[start + 3].normal.y = vertexDataSphere[start + 3].position.y;
			vertexDataSphere[start + 3].normal.z = vertexDataSphere[start + 3].position.z;

			//b(左上)
			vertexDataSphere[start + 4].position.x = cosf(lat + kLatEvery) * cosf(lon);
			vertexDataSphere[start + 4].position.y = sinf(lat + kLatEvery);
			vertexDataSphere[start + 4].position.z = cosf(lat + kLatEvery) * sinf(lon);
			vertexDataSphere[start + 4].position.w = 1.0f;
			vertexDataSphere[start + 4].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) - space };
			vertexDataSphere[start + 4].normal.x = vertexDataSphere[start + 4].position.x;
			vertexDataSphere[start + 4].normal.y = vertexDataSphere[start + 4].position.y;
			vertexDataSphere[start + 4].normal.z = vertexDataSphere[start + 4].position.z;

			//d(右上)
			vertexDataSphere[start + 5].position.x = cosf(lat + kLatEvery) * cosf(lon + kLonEvery);
			vertexDataSphere[start + 5].position.y = sinf(lat + kLatEvery);
			vertexDataSphere[start + 5].position.z = cosf(lat + kLatEvery) * sinf(lon + kLonEvery);
			vertexDataSphere[start + 5].position.w = 1.0f;
			vertexDataSphere[start + 5].texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) - space };
			vertexDataSphere[start + 5].normal.x = vertexDataSphere[start + 5].position.x;
			vertexDataSphere[start + 5].normal.y = vertexDataSphere[start + 5].position.y;
			vertexDataSphere[start + 5].normal.z = vertexDataSphere[start + 5].position.z;

		}
	}

	//UtahTeapotモデル読み込み
	ModelData modelDataTeapot = LoadObjFile("resources", "teapot.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < vertexResourcesTeapot.size(); i++) {
		vertexResourcesTeapot[i] = CreateBufferResource(device, sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsTeapot(modelDataTeapot.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViewsTeapot.size(); i++) {
		vertexBufferViewsTeapot[i].BufferLocation = vertexResourcesTeapot[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViewsTeapot[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViewsTeapot[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatasTeapot.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasTeapot[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatasTeapot[i], modelDataTeapot.mesh[i].vertices.data(), sizeof(VertexData) * modelDataTeapot.mesh[i].vertices.size());
	}

	//Stanford Bunnyモデル読み込み
	ModelData modelDataBunny = LoadObjFile("resources", "bunny.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < vertexResourcesBunny.size(); i++) {
		vertexResourcesBunny[i] = CreateBufferResource(device, sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsBunny(modelDataBunny.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViewsBunny.size(); i++) {
		vertexBufferViewsBunny[i].BufferLocation = vertexResourcesBunny[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViewsBunny[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViewsBunny[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatasBunny(modelDataBunny.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatasBunny.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasBunny[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatasBunny[i], modelDataBunny.mesh[i].vertices.data(), sizeof(VertexData) * modelDataBunny.mesh[i].vertices.size());
	}

	//Multi Meshモデル読み込み
	ModelData modelDataMultiMesh = LoadObjFile("resources", "multiMesh.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < vertexResourcesMultiMesh.size(); i++) {
		vertexResourcesMultiMesh[i] = CreateBufferResource(device, sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViewsMultiMesh.size(); i++) {
		vertexBufferViewsMultiMesh[i].BufferLocation = vertexResourcesMultiMesh[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViewsMultiMesh[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViewsMultiMesh[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatasMultiMesh.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasMultiMesh[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatasMultiMesh[i], modelDataMultiMesh.mesh[i].vertices.data(), sizeof(VertexData) * modelDataMultiMesh.mesh[i].vertices.size());
	}

	//Multi Materialモデル読み込み
	ModelData modelDataMultiMaterial = LoadObjFile("resources", "multiMaterial.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < vertexResourcesMultiMaterial.size(); i++) {
		vertexResourcesMultiMaterial[i] = CreateBufferResource(device, sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViewsMultiMaterial.size(); i++) {
		vertexBufferViewsMultiMaterial[i].BufferLocation = vertexResourcesMultiMaterial[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViewsMultiMaterial[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViewsMultiMaterial[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatasMultiMaterial.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasMultiMaterial[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatasMultiMaterial[i], modelDataMultiMaterial.mesh[i].vertices.data(), sizeof(VertexData) * modelDataMultiMaterial.mesh[i].vertices.size());
	}

	//Suzanne モデル読み込み
	ModelData modelDataSuzanne = LoadObjFile("resources", "suzanne.obj");
	//メッシュの分だけ頂点リソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResourcesSuzanne(modelDataSuzanne.mesh.size());
	for (int32_t i = 0; i < vertexResourcesSuzanne.size(); i++) {
		vertexResourcesSuzanne[i] = CreateBufferResource(device, sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());
	}
	//メッシュの分だけ頂点バッファビューを作成する
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewsSuzanne(modelDataSuzanne.mesh.size(), {});
	for (int32_t i = 0; i < vertexBufferViewsSuzanne.size(); i++) {
		vertexBufferViewsSuzanne[i].BufferLocation = vertexResourcesSuzanne[i]->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		vertexBufferViewsSuzanne[i].SizeInBytes = UINT(sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());	//使用するリソースのサイズは頂点のサイズ
		vertexBufferViewsSuzanne[i].StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
	}
	////頂点リソースにデータを書き込む
	std::vector<VertexData*> vertexDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	for (int32_t i = 0; i < vertexDatasSuzanne.size(); i++) {
		//書き込むためのアドレスを取得
		vertexResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDatasSuzanne[i]));
		//頂点データをリソースにコピー
		std::memcpy(vertexDatasSuzanne[i], modelDataSuzanne.mesh[i].vertices.data(), sizeof(VertexData) * modelDataSuzanne.mesh[i].vertices.size());
	}

	// Meshの分だけマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResources(modelData.mesh.size());
	for (int32_t i = 0; i < materialResources.size(); i++) {
		materialResources[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatas(modelData.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatas.size(); i++) {
		//書き込むためのアドレスを取得
		materialResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatas[i]));
		//今回は白を書き込んでみる
		materialDatas[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatas[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatas[i]->uvTransform = MakeIdentity4x4();
	}

	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//白
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = Light::None;
	//UVTransformを単位行列で初期化
	materialDataSprite->uvTransform = MakeIdentity4x4();

	//球用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = CreateBufferResource(device, sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	Material* materialDataSphere = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//白
	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialDataSphere->enableLighting = Light::HalfLambert;
	//UVTransformを単位行列で初期化
	materialDataSphere->uvTransform = MakeIdentity4x4();

	// Meshの分だけUtahTeapotマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < materialResourcesTeapot.size(); i++) {
		materialResourcesTeapot[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatasTeapot.size(); i++) {
		//書き込むためのアドレスを取得
		materialResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasTeapot[i]));
		//今回は白を書き込んでみる
		materialDatasTeapot[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatasTeapot[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatasTeapot[i]->uvTransform = MakeIdentity4x4();
	}

	// Meshの分だけStanford Bunnyマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < materialResourcesBunny.size(); i++) {
		materialResourcesBunny[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatasBunny(modelDataBunny.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatasBunny.size(); i++) {
		//書き込むためのアドレスを取得
		materialResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasBunny[i]));
		//今回は白を書き込んでみる
		materialDatasBunny[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatasBunny[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatasBunny[i]->uvTransform = MakeIdentity4x4();
	}

	// Meshの分だけMulti Meshマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < materialResourcesMultiMesh.size(); i++) {
		materialResourcesMultiMesh[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatasMultiMesh.size(); i++) {
		//書き込むためのアドレスを取得
		materialResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasMultiMesh[i]));
		//今回は白を書き込んでみる
		materialDatasMultiMesh[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatasMultiMesh[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatasMultiMesh[i]->uvTransform = MakeIdentity4x4();
	}

	// Meshの分だけMulti Materialマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < materialResourcesMultiMaterial.size(); i++) {
		materialResourcesMultiMaterial[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatasMultiMaterial.size(); i++) {
		//書き込むためのアドレスを取得
		materialResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasMultiMaterial[i]));
		//今回は白を書き込んでみる
		materialDatasMultiMaterial[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatasMultiMaterial[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatasMultiMaterial[i]->uvTransform = MakeIdentity4x4();
	}

	// Meshの分だけSuzanneマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResourcesSuzanne(modelDataSuzanne.mesh.size());
	for (int32_t i = 0; i < materialResourcesSuzanne.size(); i++) {
		materialResourcesSuzanne[i] = CreateBufferResource(device, sizeof(Material));
	}
	//マテリアルにデータを書き込む
	std::vector<Material*> materialDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	for (int32_t i = 0; i < materialDatasSuzanne.size(); i++) {
		//書き込むためのアドレスを取得
		materialResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&materialDatasSuzanne[i]));
		//今回は白を書き込んでみる
		materialDatasSuzanne[i]->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialDatasSuzanne[i]->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialDatasSuzanne[i]->uvTransform = MakeIdentity4x4();
	}

	// WVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//Sprite用のTransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();

	// 球用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSphere = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSphere = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSphere));
	//単位行列を書き込んでおく
	transformationMatrixDataSphere->WVP = MakeIdentity4x4();
	transformationMatrixDataSphere->World = MakeIdentity4x4();

	// UtahTeapot用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceTeapot = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataTeapot = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceTeapot->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataTeapot));
	//単位行列を書き込んでおく
	transformationMatrixDataTeapot->WVP = MakeIdentity4x4();
	transformationMatrixDataTeapot->World = MakeIdentity4x4();

	// Stanford Bunny用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceBunny = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataBunny = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceBunny->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataBunny));
	//単位行列を書き込んでおく
	transformationMatrixDataBunny->WVP = MakeIdentity4x4();
	transformationMatrixDataBunny->World = MakeIdentity4x4();

	// Multi Mesh用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceMultiMesh = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataMultiMesh = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceMultiMesh->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataMultiMesh));
	//単位行列を書き込んでおく
	transformationMatrixDataMultiMesh->WVP = MakeIdentity4x4();
	transformationMatrixDataMultiMesh->World = MakeIdentity4x4();

	// Multi Material用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceMultiMaterial = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataMultiMaterial = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceMultiMaterial->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataMultiMaterial));
	//単位行列を書き込んでおく
	transformationMatrixDataMultiMaterial->WVP = MakeIdentity4x4();
	transformationMatrixDataMultiMaterial->World = MakeIdentity4x4();

	// Suzanne用のWVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSuzanne = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSuzanne = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSuzanne->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSuzanne));
	//単位行列を書き込んでおく
	transformationMatrixDataSuzanne->WVP = MakeIdentity4x4();
	transformationMatrixDataSuzanne->World = MakeIdentity4x4();

	//DirectionalLight用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	//データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//デフォルト値はとりあえず以下のようにしておく
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	//plane用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResources(modelData.mesh.size());
	for (int32_t i = 0; i < indexResources.size(); i++) {
		indexResources[i] = CreateBufferResource(device, sizeof(uint32_t) * modelData.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViews(modelData.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViews.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViews[i].BufferLocation = indexResources[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViews[i].SizeInBytes = UINT(sizeof(uint32_t) * modelData.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//plane用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatas(modelData.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatas.size(); i++) {
		indexResources[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatas[i]));
		for (uint32_t j = 0; j < modelData.mesh[i].vertices.size(); j++) {
			indexDatas[i][j] = j;
		}
	}

	//球用のindexリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSphere = CreateBufferResource(device, sizeof(uint32_t) * vertexTotalNumber);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSphere.BufferLocation = indexResourceSphere->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	indexBufferViewSphere.SizeInBytes = UINT(sizeof(uint32_t) * vertexTotalNumber);
	//インデックスはuint32_tとする
	indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;
	//球用インデックスリソースにデータを書き込む
	uint32_t* indexDataSphere = nullptr;
	indexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = float(-M_PI) / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//現在の経度
			indexDataSphere[start] = start;
			indexDataSphere[start + 1] = start + 1;
			indexDataSphere[start + 2] = start + 2;
			indexDataSphere[start + 3] = start + 1;
			indexDataSphere[start + 4] = start + 5;
			indexDataSphere[start + 5] = start + 2;
		}
	}

	//Sprite用のindexリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//Sprite用インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	//UtahTeapot用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < indexResourcesTeapot.size(); i++) {
		indexResourcesTeapot[i] = CreateBufferResource(device, sizeof(uint32_t) * modelDataTeapot.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsTeapot(modelDataTeapot.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViewsTeapot.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViewsTeapot[i].BufferLocation = indexResourcesTeapot[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViewsTeapot[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataTeapot.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViewsTeapot[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//UtahTeapot用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatasTeapot(modelDataTeapot.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatasTeapot.size(); i++) {
		indexResourcesTeapot[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasTeapot[i]));
		for (uint32_t j = 0; j < modelDataTeapot.mesh[i].vertices.size(); j++) {
			indexDatasTeapot[i][j] = j;
		}
	}

	//Stanford Bunny用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < indexResourcesBunny.size(); i++) {
		indexResourcesBunny[i] = CreateBufferResource(device, sizeof(uint32_t) * modelDataBunny.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsBunny(modelDataBunny.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViewsBunny.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViewsBunny[i].BufferLocation = indexResourcesBunny[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViewsBunny[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataBunny.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViewsBunny[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//Stanford Bunny用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatasBunny(modelDataBunny.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatasBunny.size(); i++) {
		indexResourcesBunny[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasBunny[i]));
		for (uint32_t j = 0; j < modelDataBunny.mesh[i].vertices.size(); j++) {
			indexDatasBunny[i][j] = j;
		}
	}

	//Multi Mesh用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < indexResourcesMultiMesh.size(); i++) {
		indexResourcesMultiMesh[i] = CreateBufferResource(device, sizeof(uint32_t) * modelDataMultiMesh.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViewsMultiMesh.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViewsMultiMesh[i].BufferLocation = indexResourcesMultiMesh[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViewsMultiMesh[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataMultiMesh.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViewsMultiMesh[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//Multi Mesh用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatasMultiMesh(modelDataMultiMesh.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatasMultiMesh.size(); i++) {
		indexResourcesMultiMesh[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasMultiMesh[i]));
		for (uint32_t j = 0; j < modelDataMultiMesh.mesh[i].vertices.size(); j++) {
			indexDatasMultiMesh[i][j] = j;
		}
	}

	//Multi Material用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < indexResourcesMultiMaterial.size(); i++) {
		indexResourcesMultiMaterial[i] = CreateBufferResource(device, sizeof(uint32_t) * modelDataMultiMaterial.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViewsMultiMaterial.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViewsMultiMaterial[i].BufferLocation = indexResourcesMultiMaterial[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViewsMultiMaterial[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataMultiMaterial.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViewsMultiMaterial[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//Multi Material用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatasMultiMaterial(modelDataMultiMaterial.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatasMultiMaterial.size(); i++) {
		indexResourcesMultiMaterial[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasMultiMaterial[i]));
		for (uint32_t j = 0; j < modelDataMultiMaterial.mesh[i].vertices.size(); j++) {
			indexDatasMultiMaterial[i][j] = j;
		}
	}

	//Suzanne用のindexリソース
	//メッシュの分だけindexリソースを作る
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResourcesSuzanne(modelDataSuzanne.mesh.size());
	for (int32_t i = 0; i < indexResourcesSuzanne.size(); i++) {
		indexResourcesSuzanne[i] = CreateBufferResource(device, sizeof(uint32_t) * modelDataSuzanne.mesh[i].vertices.size());
	}
	//メッシュの分だけindexバッファビューを作る
	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferViewsSuzanne(modelDataSuzanne.mesh.size(), {});
	for (int32_t i = 0; i < indexBufferViewsSuzanne.size(); i++) {
		//リソースの先頭のアドレスから使う
		indexBufferViewsSuzanne[i].BufferLocation = indexResourcesSuzanne[i]->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		indexBufferViewsSuzanne[i].SizeInBytes = UINT(sizeof(uint32_t) * modelDataSuzanne.mesh[i].vertices.size());
		//インデックスはuint32_tとする
		indexBufferViewsSuzanne[i].Format = DXGI_FORMAT_R32_UINT;
	}
	//Suzanne用インデックスリソースにデータを書き込む
	std::vector<uint32_t*> indexDatasSuzanne(modelDataSuzanne.mesh.size(), nullptr);
	for (int32_t i = 0; i < indexDatasSuzanne.size(); i++) {
		indexResourcesSuzanne[i]->Map(0, nullptr, reinterpret_cast<void**>(&indexDatasSuzanne[i]));
		for (uint32_t j = 0; j < modelDataSuzanne.mesh[i].vertices.size(); j++) {
			indexDatasSuzanne[i][j] = j;
		}
	}

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 0),
		GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 0));

	//Transform変数を作る
	struct Transform transform { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformSprite { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformSphere { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformTeapot { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformBunny { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformMultiMesh { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformMultiMaterial { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	struct Transform transformSuzanne { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

	//UVTransform変数を作る
	struct Transform uvTransformSprite {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	struct Transform uvTransformMultiMaterial1 {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	struct Transform uvTransformMultiMaterial2 {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	//カメラ用変数を作る
	struct Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -10.0f } };

	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, device, commandList);

	//モデル用のTextureを読んで転送する
	std::vector<DirectX::ScratchImage> mipImagesModel(modelData.mesh.size());
	for (int32_t i = 0; i < mipImagesModel.size(); i++) {
		mipImagesModel[i] = LoadTexture(modelData.mesh[i].material.textureFilePath);
	}
	const  DirectX::TexMetadata& metadatasModel = mipImagesModel[0].GetMetadata();
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesModel(modelData.mesh.size());
	for (int32_t i = 0; i < textureResourcesModel.size(); i++) {
		textureResourcesModel[i] = CreateTextureResource(device, metadatasModel);
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesModel(modelData.mesh.size());
	for (int32_t i = 0; i < intermediateResourcesModel.size(); i++) {
		intermediateResourcesModel[i] = UploadTextureData(textureResourcesModel[i].Get(), mipImagesModel[i], device, commandList);
	}

	//Utah TeapotのTextureを読んで転送する
	std::vector<DirectX::ScratchImage> mipImagesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < mipImagesTeapot.size(); i++) {
		mipImagesTeapot[i] = LoadTexture(modelDataTeapot.mesh[i].material.textureFilePath);
	}
	const DirectX::TexMetadata& metadatasTeapot = mipImagesTeapot[0].GetMetadata();
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < textureResourcesTeapot.size(); i++) {
		textureResourcesTeapot[i] = CreateTextureResource(device, metadatasTeapot);
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesTeapot(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < intermediateResourcesTeapot.size(); i++) {
		intermediateResourcesTeapot[i] = UploadTextureData(textureResourcesTeapot[i].Get(), mipImagesTeapot[i], device, commandList);
	}

	//Stanford BunnyのTextureを読んで転送する
	std::vector<DirectX::ScratchImage> mipImagesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < mipImagesBunny.size(); i++) {
		mipImagesBunny[i] = LoadTexture(modelDataBunny.mesh[i].material.textureFilePath);
	}
	const DirectX::TexMetadata& metadatasBunny = mipImagesBunny[0].GetMetadata();
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < textureResourcesBunny.size(); i++) {
		textureResourcesBunny[i] = CreateTextureResource(device, metadatasBunny);
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesBunny(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < intermediateResourcesBunny.size(); i++) {
		intermediateResourcesBunny[i] = UploadTextureData(textureResourcesBunny[i].Get(), mipImagesBunny[i], device, commandList);
	}

	//Multi MeshのTextureを読んで転送する
	std::vector<DirectX::ScratchImage> mipImagesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < mipImagesMultiMesh.size(); i++) {
		mipImagesMultiMesh[i] = LoadTexture(modelDataMultiMesh.mesh[i].material.textureFilePath);
	}
	//const DirectX::TexMetadata& metadatasMultiMesh = mipImagesMultiMesh[0].GetMetadata();
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < textureResourcesMultiMesh.size(); i++) {
		textureResourcesMultiMesh[i] = CreateTextureResource(device, mipImagesMultiMesh[i].GetMetadata());
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesMultiMesh(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < intermediateResourcesMultiMesh.size(); i++) {
		intermediateResourcesMultiMesh[i] = UploadTextureData(textureResourcesMultiMesh[i].Get(), mipImagesMultiMesh[i], device, commandList);
	}

	//Multi Material用のTextureを読んで転送する
	std::vector<DirectX::ScratchImage> mipImagesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < mipImagesMultiMaterial.size(); i++) {
		mipImagesMultiMaterial[i] = LoadTexture(modelDataMultiMaterial.mesh[i].material.textureFilePath);
	}
	//const  DirectX::TexMetadata& metadatasMultiMaterial = mipImagesMultiMaterial[0].GetMetadata();
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < textureResourcesMultiMaterial.size(); i++) {
		textureResourcesMultiMaterial[i] = CreateTextureResource(device, mipImagesMultiMaterial[i].GetMetadata());
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResourcesMultiMaterial(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < intermediateResourcesMultiMaterial.size(); i++) {
		intermediateResourcesMultiMaterial[i] = UploadTextureData(textureResourcesMultiMaterial[i].Get(), mipImagesMultiMaterial[i], device, commandList);
	}

	//SRVの総数を記録しておく
	int32_t SRVCount{};
	
	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	//先頭はImGuiが使っているのでその次を使う
	SRVCount++;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);
	//SRVの生成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	//metaDataを基にSRVの設定2
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsModel(modelData.mesh.size(), {});
	for (int32_t i = 0; i < srvDescsModel.size(); i++) {
		srvDescsModel[i].Format = metadatasModel.format;
		srvDescsModel[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescsModel[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDescsModel[i].Texture2D.MipLevels = UINT(metadatasModel.mipLevels);
	}
	//SRVを作成するDescriptorHeapの位置を決める2
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesModelCPU(modelData.mesh.size());
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesModelGPU(modelData.mesh.size());
	for (int32_t i = 0; i < modelData.mesh.size(); i++) {
		SRVCount++;
		textureSrvHandlesModelCPU[i] = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
		textureSrvHandlesModelGPU[i] = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
	}
	//SRVの生成2
	for (int32_t i = 0; i < modelData.mesh.size(); i++) {
		device->CreateShaderResourceView(textureResourcesModel[i].Get(), &srvDescsModel[i], textureSrvHandlesModelCPU[i]);
	}

	//metaDataを基にSRVの設定Teapot
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsTeapot(modelDataTeapot.mesh.size(), {});
	for (int32_t i = 0; i < srvDescsTeapot.size(); i++) {
		srvDescsTeapot[i].Format = metadatasTeapot.format;
		srvDescsTeapot[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescsTeapot[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDescsTeapot[i].Texture2D.MipLevels = UINT(metadatasTeapot.mipLevels);
	}
	//SRVを作成するDescriptorHeapの位置を決めるTeapot
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesTeapotCPU(modelDataTeapot.mesh.size());
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesTeapotGPU(modelDataTeapot.mesh.size());
	for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
		SRVCount++;
		textureSrvHandlesTeapotCPU[i] = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
		textureSrvHandlesTeapotGPU[i] = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
	}
	//SRVの生成Teapot
	for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
		device->CreateShaderResourceView(textureResourcesTeapot[i].Get(), &srvDescsTeapot[i], textureSrvHandlesTeapotCPU[i]);
	}

	//metaDataを基にSRVの設定Stanford Bunny
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsBunny(modelDataBunny.mesh.size(), {});
	for (int32_t i = 0; i < srvDescsBunny.size(); i++) {
		srvDescsBunny[i].Format = metadatasBunny.format;
		srvDescsBunny[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescsBunny[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDescsBunny[i].Texture2D.MipLevels = UINT(metadatasBunny.mipLevels);
	}
	//SRVを作成するDescriptorHeapの位置を決めるStanford Bunny
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesBunnyCPU(modelDataBunny.mesh.size());
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesBunnyGPU(modelDataBunny.mesh.size());
	for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
		SRVCount++;
		textureSrvHandlesBunnyCPU[i] = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
		textureSrvHandlesBunnyGPU[i] = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
	}
	//SRVの生成Stanford Bunny
	for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
		device->CreateShaderResourceView(textureResourcesBunny[i].Get(), &srvDescsBunny[i], textureSrvHandlesBunnyCPU[i]);
	}

	//metaDataを基にSRVの設定Multi Mesh
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsMultiMesh(modelDataMultiMesh.mesh.size(), {});
	for (int32_t i = 0; i < srvDescsMultiMesh.size(); i++) {
		srvDescsMultiMesh[i].Format = mipImagesMultiMesh[i].GetMetadata().format;
		srvDescsMultiMesh[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescsMultiMesh[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDescsMultiMesh[i].Texture2D.MipLevels = UINT(mipImagesMultiMesh[i].GetMetadata().mipLevels);
	}
	//SRVを作成するDescriptorHeapの位置を決めるMulti Mesh
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMeshCPU(modelDataMultiMesh.mesh.size());
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMeshGPU(modelDataMultiMesh.mesh.size());
	for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
		SRVCount++;
		textureSrvHandlesMultiMeshCPU[i] = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
		textureSrvHandlesMultiMeshGPU[i] = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
	}
	//SRVの生成Multi Mesh
	for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
		device->CreateShaderResourceView(textureResourcesMultiMesh[i].Get(), &srvDescsMultiMesh[i], textureSrvHandlesMultiMeshCPU[i]);
	}

	//metaDataを基にSRVの設定Multi Material
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescsMultiMaterial(modelDataMultiMaterial.mesh.size(), {});
	for (int32_t i = 0; i < srvDescsMultiMaterial.size(); i++) {
		srvDescsMultiMaterial[i].Format = mipImagesMultiMaterial[i].GetMetadata().format;
		srvDescsMultiMaterial[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescsMultiMaterial[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDescsMultiMaterial[i].Texture2D.MipLevels = UINT(mipImagesMultiMaterial[i].GetMetadata().mipLevels);
	}
	//SRVを作成するDescriptorHeapの位置を決めるMulti Material
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMaterialCPU(modelDataMultiMaterial.mesh.size());
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesMultiMaterialGPU(modelDataMultiMaterial.mesh.size());
	for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
		SRVCount++;
		textureSrvHandlesMultiMaterialCPU[i] = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
		textureSrvHandlesMultiMaterialGPU[i] = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, SRVCount);
	}
	//SRVの生成Multi Material
	for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
		device->CreateShaderResourceView(textureResourcesMultiMaterial[i].Get(), &srvDescsMultiMaterial[i], textureSrvHandlesMultiMaterialCPU[i]);
	}

	//DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	//2dTexture
	//DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, GetCPUDescriptorHandle(dsvDescriptorHeap.Get(), descriptorSizeDSV, 0));

	//音声読み込み
	SoundData soundData1 = SoundLoadWave("resources/Alarm01.wav");

	BYTE beforeKey[256] = {};
	
	DebugCamera* debugcamera = new DebugCamera();
	debugcamera->Initialize(kClientWidth, kClientHeight);
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

	MSG msg{};
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowsにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//キーボード情報の取得開始
			keyboard->Acquire();

			//全キーの入力状態を取得する
			BYTE key[256] = {};
			keyboard->GetDeviceState(sizeof(key), key);

			if (gamepad) {
				//ゲームパッド情報の取得開始
				gamepad->Acquire();

				//入力状態を取得する
				DIJOYSTATE padKey;
				gamepad->GetDeviceState(sizeof(DIJOYSTATE), &padKey);
			}

			#ifdef _DEBUG
				if(GetKeyDown(key, beforeKey, DIK_V)) {
					useDebugcamera = !useDebugcamera;
				}

				if (useDebugcamera) {
					debugcamera->Update(key);
				}
			#endif

			//ゲームの処理
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix;
			if (useDebugcamera) {
				worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixData->WVP = worldViewProjectionMatrix;
			transformationMatrixData->World = worldMatrix;

			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->World = worldMatrixSprite;

			//球の処理
			Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
			Matrix4x4 worldViewProjectionMatrixSphere;
			if (useDebugcamera) {
				worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataSphere->WVP = worldViewProjectionMatrixSphere;
			transformationMatrixDataSphere->World = worldMatrixSphere;

			//Utah Teapotの処理
			Matrix4x4 worldMatrixTeapot = MakeAffineMatrix(transformTeapot.scale, transformTeapot.rotate, transformTeapot.translate);
			Matrix4x4 worldViewProjectionMatrixTeapot;
			if (useDebugcamera) {
				worldViewProjectionMatrixTeapot = Multiply(worldMatrixTeapot, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixTeapot = Multiply(worldMatrixTeapot, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataTeapot->WVP = worldViewProjectionMatrixTeapot;
			transformationMatrixDataTeapot->World = worldMatrixTeapot;

			//Stanford Bunnyの処理
			Matrix4x4 worldMatrixBunny = MakeAffineMatrix(transformBunny.scale, transformBunny.rotate, transformBunny.translate);
			Matrix4x4 worldViewProjectionMatrixBunny;
			if (useDebugcamera) {
				worldViewProjectionMatrixBunny = Multiply(worldMatrixBunny, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixBunny = Multiply(worldMatrixBunny, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataBunny->WVP = worldViewProjectionMatrixBunny;
			transformationMatrixDataBunny->World = worldMatrixBunny;

			//Multi Meshの処理
			Matrix4x4 worldMatrixMultiMesh = MakeAffineMatrix(transformMultiMesh.scale, transformMultiMesh.rotate, transformMultiMesh.translate);
			Matrix4x4 worldViewProjectionMatrixMultiMesh;
			if (useDebugcamera) {
				worldViewProjectionMatrixMultiMesh = Multiply(worldMatrixMultiMesh, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixMultiMesh = Multiply(worldMatrixMultiMesh, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataMultiMesh->WVP = worldViewProjectionMatrixMultiMesh;
			transformationMatrixDataMultiMesh->World = worldMatrixMultiMesh;

			//Multi Materialの処理
			Matrix4x4 worldMatrixMultiMaterial = MakeAffineMatrix(transformMultiMaterial.scale, transformMultiMaterial.rotate, transformMultiMaterial.translate);
			Matrix4x4 worldViewProjectionMatrixMultiMaterial;
			if (useDebugcamera) {
				worldViewProjectionMatrixMultiMaterial = Multiply(worldMatrixMultiMaterial, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixMultiMaterial = Multiply(worldMatrixMultiMaterial, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataMultiMaterial->WVP = worldViewProjectionMatrixMultiMaterial;
			transformationMatrixDataMultiMaterial->World = worldMatrixMultiMaterial;

			//Suzanneの処理
			Matrix4x4 worldMatrixSuzanne = MakeAffineMatrix(transformSuzanne.scale, transformSuzanne.rotate, transformSuzanne.translate);
			Matrix4x4 worldViewProjectionMatrixSuzanne;
			if (useDebugcamera) {
				worldViewProjectionMatrixSuzanne = Multiply(worldMatrixSuzanne, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
			} else {
				worldViewProjectionMatrixSuzanne = Multiply(worldMatrixSuzanne, Multiply(viewMatrix, projectionMatrix));
			}
			transformationMatrixDataSuzanne->WVP = worldViewProjectionMatrixSuzanne;
			transformationMatrixDataSuzanne->World = worldMatrixSuzanne;

			//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			ImGui::Begin("ImGui");
			if (ImGui::TreeNode("Sprite")) {
				ImGui::Checkbox("drawSprite", &drawSprite);
				ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transformSprite.scale), -5, 5);
				ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transformSprite.rotate), -5, 5);
				ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transformSprite.translate), 0, 1000);
				ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
				ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
				ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("plane.obj")) {
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
			ImGui::End();

			if (playSound) {
				//音声再生
				SoundPlayWave(xAudio2.Get(), soundData1);
				playSound = false;
			}

			directionalLightData->direction = Normalize(directionalLightData->direction);

			//パラメータからUVTransform用の行列を生成する
			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			Matrix4x4 uvTransformMatrixMultiMaterial1 = MakeScaleMatrix(uvTransformMultiMaterial1.scale);
			uvTransformMatrixMultiMaterial1 = Multiply(uvTransformMatrixMultiMaterial1, MakeRotateZMatrix(uvTransformMultiMaterial1.rotate.z));
			uvTransformMatrixMultiMaterial1 = Multiply(uvTransformMatrixMultiMaterial1, MakeTranslateMatrix(uvTransformMultiMaterial1.translate));
			materialDatasMultiMaterial[0]->uvTransform = uvTransformMatrixMultiMaterial1;

			Matrix4x4 uvTransformMatrixMultiMaterial2 = MakeScaleMatrix(uvTransformMultiMaterial2.scale);
			uvTransformMatrixMultiMaterial2 = Multiply(uvTransformMatrixMultiMaterial2, MakeRotateZMatrix(uvTransformMultiMaterial2.rotate.z));
			uvTransformMatrixMultiMaterial2 = Multiply(uvTransformMatrixMultiMaterial2, MakeTranslateMatrix(uvTransformMultiMaterial2.translate));
			materialDatasMultiMaterial[1]->uvTransform = uvTransformMatrixMultiMaterial2;

			//ImGuiの内部コマンドを生成する
			ImGui::Render();

			//これから書き込むバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			//TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
			//遷移前(現在)のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			//描画先のRTVとDSVを設定する
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvDescriptorHeap.Get(), descriptorSizeDSV, 0);
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			//指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	//青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
			//指定した深度で画面全体をクリアする
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			//描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get()};
			commandList->SetDescriptorHeaps(1, descriptorHeaps);
			
			commandList->RSSetViewports(1, &viewport);	//Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);	//Scissorを設定
			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			commandList->SetGraphicsRootSignature(rootSignature.Get());
			commandList->SetPipelineState(graphicsPipelineState.Get());	//PSOを設定
			//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//SRVのDescroptorTableの先頭を設定。2はrootParameter[2]である。
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesModelGPU[0]);
			//directionalLight用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

			for (int32_t i = 0; i < modelData.mesh.size(); i++) {
				//VBVを設定
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViews[i]);
				//wvp用のCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResources[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViews[i]);
				//描画！ (DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
				if (drawPlane) {
					commandList->DrawIndexedInstanced(UINT(modelData.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			//球の描画。変更が必要なものだけ変更する
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);	//VBVを設定
			//TransformationMatrixCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSphere->GetGPUVirtualAddress());
			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
			//IBVを設定
			commandList->IASetIndexBuffer(&indexBufferViewSphere);
			if (drawSphere) {
				commandList->DrawIndexedInstanced(vertexTotalNumber, 1, 0, 0, 0);
			}

			for (int32_t i = 0; i < modelDataTeapot.mesh.size(); i++) {
				//Utah Teapotのテクスチャ
				commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesTeapotGPU[i]);
				//Utah Teapotの描画。変更が必要なものだけ変更する
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViewsTeapot[i]);	//VBVを設定
				//TransformationMatrixCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceTeapot->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResourcesTeapot[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViewsTeapot[i]);
				if (drawTeapot) {
					commandList->DrawIndexedInstanced(UINT(modelDataTeapot.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			for (int32_t i = 0; i < modelDataBunny.mesh.size(); i++) {
				//Stanford Bunnyのテクスチャ
				commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesBunnyGPU[i]);
				//Stanford Bunnyの描画。変更が必要なものだけ変更する
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViewsBunny[i]);	//VBVを設定
				//TransformationMatrixCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceBunny->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResourcesBunny[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViewsBunny[i]);
				if (drawBunny) {
					commandList->DrawIndexedInstanced(UINT(modelDataBunny.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			for (int32_t i = 0; i < modelDataMultiMesh.mesh.size(); i++) {
				//Multi Meshのテクスチャ
				commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesMultiMeshGPU[i]);
				//Multi Meshの描画。変更が必要なものだけ変更する
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViewsMultiMesh[i]);	//VBVを設定
				//TransformationMatrixCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceMultiMesh->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResourcesMultiMesh[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViewsMultiMesh[i]);
				if (drawMultiMesh) {
					commandList->DrawIndexedInstanced(UINT(modelDataMultiMesh.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			for (int32_t i = 0; i < modelDataMultiMaterial.mesh.size(); i++) {
				//Multi Meshのテクスチャ
				commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesMultiMaterialGPU[i]);
				//Multi Meshの描画。変更が必要なものだけ変更する
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViewsMultiMaterial[i]);	//VBVを設定
				//TransformationMatrixCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceMultiMaterial->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResourcesMultiMaterial[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViewsMultiMaterial[i]);
				if (drawMultiMaterial) {
					commandList->DrawIndexedInstanced(UINT(modelDataMultiMaterial.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			for (int32_t i = 0; i < modelDataSuzanne.mesh.size(); i++) {
				//Suzanneはテクスチャなし
				//Suzanneの描画。変更が必要なものだけ変更する
				commandList->IASetVertexBuffers(0, 1, &vertexBufferViewsSuzanne[i]);	//VBVを設定
				//TransformationMatrixCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSuzanne->GetGPUVirtualAddress());
				//マテリアルCBufferの場所を設定
				commandList->SetGraphicsRootConstantBufferView(0, materialResourcesSuzanne[i]->GetGPUVirtualAddress());
				//IBVを設定
				commandList->IASetIndexBuffer(&indexBufferViewsSuzanne[i]);
				if (drawSuzanne) {
					commandList->DrawIndexedInstanced(UINT(modelDataSuzanne.mesh[i].vertices.size()), 1, 0, 0, 0);
				}
			}

			//Spriteは常にuvCheckerにする
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			//Spriteの描画。変更が必要なものだけ変更する
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);	//VBVを設定
			//TransformationMatrixCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			//IBVを設定
			commandList->IASetIndexBuffer(&indexBufferViewSprite);
			//描画！(DrawCall/ドローコール)
			if (drawSprite) {
				commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
			}

			//実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

			//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
			//今回はRenderTargetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList.Get()};
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うよう通知する
			swapChain->Present(1, 0);

			//Fenceの値を更新
			fenceValue++;
			//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence.Get(), fenceValue);
			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompletedValueの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue) {
				//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベント待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator.Get(), nullptr);
			assert(SUCCEEDED(hr));

			//キー入力状態をコピー
			for (int i = 0; i < 256; i++) {
				beforeKey[i] = key[i];
			}
		}
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
	CloseHandle(fenceEvent);
	CloseWindow(hwnd);
	//XAudio2解放
	xAudio2.Reset();
	//音声データ解放
	SoundUnload(&soundData1);

	return 0;
}