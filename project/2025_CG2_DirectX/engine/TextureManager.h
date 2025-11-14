#pragma once
#include <d3d12.h>
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "DirectXBasic.h"
#include <vector>
#include <unordered_map>
#include "SRVManager.h"

class TextureManager
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="directXBasic">DirectXの基盤</param>
	void Initialize(DirectXBasic* directXBasic, SRVManager* srvManager);

	/// <summary>
	///	テクスチャを読み込んで使用可能な状態にする
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadTexture(const std::string& filePath);

	/// <summary>
	///	ファイルパスからGPUハンドルを取得
	/// </summary>
	/// <param name="filePath">テクスチャのファイルパス</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	/// <summary>
	///	ファイルパスからメタデータを取得
	/// </summary>
	/// <param name="filePath">テクスチャのファイルパス</param>
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

private:

	/* --------- private変数 --------- */

	struct TextureData {
		// テクスチャリソース
		Comptr<ID3D12Resource> textureResource_ = nullptr;
		uint32_t srvIndex_;
		// テクスチャのSRVハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_{};
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
		// メタデータ
		DirectX::TexMetadata metadata_;
		// 中間リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_;
	};

	// DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// SRVマネージャーのポインタ
	SRVManager* srvManager_ = nullptr;

	// テクスチャデータのmap
	std::unordered_map<std::string, TextureData> textureDatas_;

};

