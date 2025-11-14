#pragma once
#include "DirectXBasic.h"
#include <d3d12.h>

class SRVManager
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- 定数 --------- */

	//SRVの上限数
	const uint32_t kMaxSRV_ = 128;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="directXBasic">DirectXの基盤</param>
	void Initialize(DirectXBasic* directXBasic);

	/// <summary>
	///	ヒープをセット
	/// </summary>
	void PreDraw();

	/// <summary>
	///	Texture2D用のSRVを生成
	/// </summary>
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

	/// <summary>
	///	StructuredBuffer用のSRVを生成
	/// </summary>
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	/// <summary>
	///	SRVヒープをセット
	/// </summary>
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	/// <summary>
	///	SRV確保可能チェック
	/// </summary>
	/// <param name="index">テクスチャ枚数</param>
	bool AllocateRimitChack(uint32_t index);

	/// <summary>
	///	SRVインデックスを確保
	/// </summary>
	uint32_t Allocate();

	/// <summary>
	/// SRVのCPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="index">インデックス</param>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVのGPUディスクリプタハンドルを取得
	/// </summary>
	/// <param name="index">インデックス</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	/* --------- ゲッター --------- */

	/// <summary>
	/// ディスクリプタヒープのゲッター
	/// </summary>
	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap_.Get(); };

private:

	/* --------- private変数 --------- */

	// DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// SRV用のヒープ
	Comptr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
	// ディスクリプターのサイズ
	uint32_t descriptorSize_;

	// 使われているSRVインデックス
	uint32_t useIndex_ = 0;

};

