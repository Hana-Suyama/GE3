#pragma once
#include "../DirectXBasic.h"
#include "../../../VertexData.h"
#include "../TextureManager.h"

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex;
};

struct Mesh {
	std::vector<VertexData> vertices;
	MaterialData material;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
};

struct ModelData {
	std::vector<Mesh> meshes;
	std::string mtlFileName;
	//ファイルパス
	std::string filePath;
};

class ModelManager
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
	void Initialize(DirectXBasic* directXBasic, TextureManager* textureManager);

	/// <summary>
	///	モデルを読み込んで使用可能な状態にする
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	void LoadModel(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	///	モデルの要素番号を返す
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	uint32_t GetModelIndexByFilePath(const std::string& filePath);

	ModelData* GetModelPointer(uint32_t index) { return &modelDatas_.at(index); }

	TextureManager* GetTextureManager() const { return textureManager_; }

private:

	/* --------- private変数 --------- */


	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName);

	//	DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// テクスチャマネージャのポインタ
	TextureManager* textureManager_ = nullptr;

	std::vector<ModelData> modelDatas_;

	//モデルデータの読み込み上限数
	const uint32_t kModelMax_ = 128;
};

