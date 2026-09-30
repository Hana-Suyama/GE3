#pragma once
#include "Model.h"

#include <cstdint>
#include <string>
#include <vector>

struct aiNode;
struct Skeleton;
class DirectXBasic;
class SRVManager;
class TextureManager;

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
	void Initialize(DirectXBasic* directXBasic, TextureManager* textureManager, SRVManager* srvManager);

	/// <summary>
	///	モデルを読み込んで使用可能な状態にする
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	void LoadModel(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	///	モデルをAssimpで読み込んで使用可能な状態にする
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	void LoadModelAssimp(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// 球のモデルを構築
	/// </summary>
	void CreateSphere();

	/// <summary>
	/// スカイボックスのモデルを構築
	/// </summary>
	void CreateSkyBox();

	/// <summary>
	///	モデルの要素番号を返す
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	uint32_t GetModelIndexByFilePath(const std::string& filePath);

	Model* GetModelPointer(uint32_t index) { return &modelDatas_.at(index); }

	TextureManager* GetTextureManager() const { return textureManager_; }

	Model::Node ReadNode(aiNode* node);

	Model::SkinCluster CreateSkinCluster(const Skeleton& skeleton, const Model::Mesh& modelData);

private:

	/* --------- private変数 --------- */


	Model LoadObjFile(const std::string& directoryPath, const std::string& filename);

	Model LoadObjFileAssimp(const std::string& directoryPath, const std::string& filename);

	std::string LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName);

	//	DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// テクスチャマネージャのポインタ
	TextureManager* textureManager_ = nullptr;

	// SRVマネージャのポインタ
	SRVManager* srvManager_ = nullptr;

	std::vector<Model> modelDatas_;

	//モデルデータの読み込み上限数
	const uint32_t kModelMax_ = 128;
};

