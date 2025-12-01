#pragma once
#include "../DirectXBasic.h"
#include "../../../VertexData.h"
#include "../TextureManager.h"
#include "Model.h"



class ModelManager
{
public:
	

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

	Model* GetModelPointer(uint32_t index) { return &modelDatas_.at(index); }

	TextureManager* GetTextureManager() const { return textureManager_; }

private:

	/* --------- private変数 --------- */


	Model LoadObjFile(const std::string& directoryPath, const std::string& filename);

	std::string LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName);

	//	DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// テクスチャマネージャのポインタ
	TextureManager* textureManager_ = nullptr;

	std::vector<Model> modelDatas_;

	//モデルデータの読み込み上限数
	const uint32_t kModelMax_ = 128;
};

