#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "utility/Logger.h"
#include "WindowsApi.h"
#include <dxcapi.h>
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "FixFPS.h"

class DirectXBasic
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	/// デクストラクタ
	/// </summary>
	~DirectXBasic();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="logger">ロガー</param>
	/// <param name="winApi">WindowsApi</param>
	void Initialize(Logger* logger, WindowsApi* winApi);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// シェーダーコンパイル
	/// </summary>
	/// <param name="filePath">CompileするShaderファイルへのパス</param>
	/// <param name="profile">Compileに使用するProfile</param>
	/// <param name="logger">ロガー</param>
	Comptr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile, Logger* logger);

	/// <summary>
	/// BufferResourceの作成
	/// </summary>
	/// <param name="sizeInBytes">リソースのサイズ</param>
	Comptr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// TextureResourceの作成
	/// </summary>
	/// <param name="metadata">メタデータ</param>
	Comptr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	/// <summary>
	/// テクスチャのアップロード
	/// </summary>
	/// <param name="texture">テクスチャ</param>
	/// <param name="mipImages">mipイメージ</param>
	Comptr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	/// <summary>
	/// テクスチャを読む
	/// </summary>
	/// <param name="filePath">テクスチャのファイルパス</param>
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	/// <summary>
	/// CPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap">ディスクリプタヒープ</param>
	/// <param name="descriptorSize">ディスクリプタのサイズ</param>
	/// <param name="index">インデックス</param>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// GPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="descriptorHeap">ディスクリプタヒープ</param>
	/// <param name="descriptorSize">ディスクリプタのサイズ</param>
	/// <param name="index">インデックス</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// ディスクリプタヒープの生成
	/// </summary>
	Comptr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	/* --------- ゲッター --------- */

	/// <summary>
	/// デバイスのゲッター
	/// </summary>
	ID3D12Device* GetDevice() const { return device.Get(); };

	/// <summary>
	/// コマンドリストのゲッター
	/// </summary>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); };

	/// <summary>
	/// フェンスイベントのゲッター
	/// </summary>
	HANDLE GetFenceEvent() const { return fenceEvent; };

	/// <summary>
	/// ディスクリプタヒープのゲッター
	/// </summary>
	//Comptr<ID3D12DescriptorHeap> GetSRVDescHeap() const { return srvDescriptorHeap; };
	Comptr<ID3D12DescriptorHeap> GetRTVDescHeap() const { return rtvDescriptorHeap; };
	Comptr<ID3D12DescriptorHeap> GetDSVDescHeap() const { return dsvDescriptorHeap; };

	/// <summary>
	/// ディスクリプタサイズのゲッター
	/// </summary>
	//const uint32_t GetSRVHeapSize() const { return descriptorSizeSRV; };
	const uint32_t GetRTVHeapSize() const { return descriptorSizeRTV; };
	const uint32_t GetDSVHeapSize() const { return descriptorSizeDSV; };

public:

	/* --------- public変数(定数) --------- */

	

private:

	/* --------- private関数 --------- */

	/// <summary>
	/// デバイスの生成
	/// </summary>
	void CreateDevice();

	/// <summary>
	/// コマンドの生成
	/// </summary>
	void CreateCommand();

	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// 全てのディスクリプタヒープの作成生成
	/// </summary>
	void CreateAllDiscriptorHeap();

	/// <summary>
	/// レンダーターゲットビューの生成
	/// </summary>
	void CreateRTV();

	/// <summary>
	/// フェンスの生成
	/// </summary>
	void CreateFence();

	/// <summary>
	/// DirectXCompilerの生成
	/// </summary>
	void CreateDXCompiler();

	/// <summary>
	/// ビューポートの生成
	/// </summary>
	void CreateViewPort();

	/// <summary>
	/// シザー矩形の生成
	/// </summary>
	void CreateScissorRect();

	/// <summary>
	/// 深度バッファの生成
	/// </summary>
	void CreateDepthBuffer();

	/// <summary>
	/// デプスステンシルビューの生成
	/// </summary>
	void CreateDSV();

	/// <summary>
	/// ImGuiの初期化
	/// </summary>
	void ImGuiIni();

private:

	/* --------- private変数 --------- */

	// WindowsApi
	WindowsApi* winApi_ = nullptr;

	//デバイス
	Comptr<ID3D12Device> device = nullptr;

	//DXGIファクトリー
	Comptr<IDXGIFactory7> dxgiFactory = nullptr;

	//コマンドリスト
	Comptr<ID3D12GraphicsCommandList> commandList = nullptr;
	//コマンドキュー
	Comptr<ID3D12CommandQueue> commandQueue = nullptr;
	//コマンドアロケータ
	Comptr<ID3D12CommandAllocator> commandAllocator = nullptr;

	//スワップチェーン
	Comptr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	Comptr<ID3D12Resource> swapChainResources[2] = { nullptr };

	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue;
	HANDLE fenceEvent;

	//バリア
	D3D12_RESOURCE_BARRIER barrier{};

	//デプスステンシルリソース
	Comptr<ID3D12Resource> depthStencilResource = nullptr;

	//RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	//DXCコンパイラー
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcUtils* dxcUtils = nullptr;

	//インクルードハンドラー
	IDxcIncludeHandler* includeHandler = nullptr;

	//シザー矩形
	D3D12_RECT scissorRect{};
	//ビューポート
	D3D12_VIEWPORT viewport{};

	//ロガー
	Logger* logger_;

	//ディスクリプターのサイズ
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	Comptr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	Comptr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	std::unique_ptr<FixFPS> fixFps_;

};

