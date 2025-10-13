#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "../../../VertexData.h"
#include "../../../Material.h"
#include "../../../TransformationMatrix.h"
#include "SpriteBasic.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

class Sprite
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="spriteBasic">Spriteの基盤</param>
	void Initialize(SpriteBasic* spriteBasic);

	/// <summary>
	///	更新
	/// </summary>
	void Update();

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

private:

	/* --------- private関数 --------- */

	/// <summary>
	///	頂点リソースを作成
	/// </summary>
	void CreateVertexResource();

	/// <summary>
	///	頂点データを作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	///	マテリアルリソースを作成
	/// </summary>
	void CreateMaterialResource();

	/// <summary>
	///	座標変換行列を作成
	/// </summary>
	void CreateTransformationMatrixResource();

	/// <summary>
	///	インデックスリソースを作成
	/// </summary>
	void CreateIndexResource();

	/// <summary>
	///	Transformを作成
	/// </summary>
	void CreateTransform();

	/// <summary>
	///	テクスチャをアップロード
	/// </summary>
	void TextureUpload();

	/// <summary>
	///	SRVを作成
	/// </summary>
	void CreateSRV();

private:

	/* --------- private変数 --------- */

	// Sprite基盤のポインタ
	SpriteBasic* spriteBasic_ = nullptr;

	// 頂点リソース
	Comptr<ID3D12Resource> vertexResource = nullptr;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// テクスチャリソース
	Comptr<ID3D12Resource> textureResource = nullptr;
	// テクスチャのSRVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};

	// Transform
	struct Transform transform{};
	// UVTransform
	struct Transform uvTransform{};

	// 座標変換行列
	TransformationMatrix* transformationMatrixData = nullptr;
	// 座標変換行列リソース
	Comptr<ID3D12Resource> transformationMatrixResource = nullptr;

	// マテリアル
	Material* materialData = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource = nullptr;

	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	// ミップイメージ
	DirectX::ScratchImage mipImages;
	
	// 中間リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;

};

