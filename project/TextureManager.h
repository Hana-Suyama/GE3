#pragma once
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include "2025_CG2_DirectX/engine/DirectXBasic.h"
#include <vector>

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
	void Initialize(DirectXBasic* directXBasic);

	/// <summary>
	///	テクスチャを読み込んで使用可能な状態にする
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadTexture(const std::string& filePath);

	/// <summary>
	///	テクスチャの要素番号を返す
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	/// <summary>
	///	要素番号からGPUハンドルを返す
	/// </summary>
	/// <param name="textureIndex">テクスチャの番号</param>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	/// <summary>
	///	メタデータを取得
	/// </summary>
	/// <param name="textureIndex">テクスチャの番号</param>
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

	/* --------- ゲッター --------- */

	const D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandleGPU(const int32_t index) const { return textureDatas.at(index).textureSrvHandleGPU; };

private:

	/* --------- private変数 --------- */

	struct TextureData {
		//ファイルパス
		std::string filePath;
		// テクスチャリソース
		Comptr<ID3D12Resource> textureResource = nullptr;
		// テクスチャのSRVハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};
		// メタデータ
		DirectX::TexMetadata metadata;
		// 中間リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	};

	/// <summary>
	///	DirectX基盤のポインタ
	/// </summary>
	DirectXBasic* directXBasic_ = nullptr;

	std::vector<TextureData> textureDatas;

	//SRVインデックスを何番から使うか。0番をImGuiに使っているため1番から始める
	const uint32_t kSRVIndexTop = 1;
};

