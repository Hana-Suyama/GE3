#include "ModelManager.h"
#include "../../../Material.h"

void ModelManager::Initialize(DirectXBasic* directXBasic, TextureManager* textureManager)
{
	directXBasic_ = directXBasic;

	textureManager_ = textureManager;
}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename)
{
	// 同じモデルが読み込まれていたら読み込まない
	auto it = std::find_if(
		modelDatas.begin(),
		modelDatas.end(),
		[&](ModelData& modelData) {return modelData.filePath == directoryPath + "/" + filename; }
	);
	if (it != modelDatas.end()) {
		return;
	}

	// モデル上限を超えて読み込もうとしたら止める
	assert(modelDatas.size() < kModelMax);

	// 新しく追加する空のモデルデータを作成
	ModelData newModel;
	// Objを読み込む
	newModel = LoadObjFile(directoryPath, filename);

	//ファイルパスを記録
	newModel.filePath = directoryPath + "/" + filename;

	// メッシュごとに頂点リソースを作る
	for (auto& mesh : newModel.meshes) {
		mesh.vertexResource = directXBasic_->CreateBufferResource(sizeof(VertexData) * mesh.vertices.size());
		//メッシュの分だけ頂点バッファビューを作成する
		mesh.vertexBufferView.BufferLocation = mesh.vertexResource->GetGPUVirtualAddress();	//リソースの先頭のアドレスから使う
		mesh.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * mesh.vertices.size());	//使用するリソースのサイズは頂点のサイズ
		mesh.vertexBufferView.StrideInBytes = sizeof(VertexData);	//1頂点当たりのサイズ
		//頂点リソースにデータを書き込む
		VertexData* vertexData = nullptr;
		//書き込むためのアドレスを取得
		mesh.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		//頂点データをリソースにコピー
		std::memcpy(vertexData, mesh.vertices.data(), sizeof(VertexData) * mesh.vertices.size());
	}

	// メッシュごとにマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	for (auto& mesh : newModel.meshes) {
		mesh.materialResource = directXBasic_->CreateBufferResource(sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialData = nullptr;
		//書き込むためのアドレスを取得
		mesh.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//今回は白を書き込んでみる
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialData->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialData->uvTransform = MyMath::MakeIdentity4x4();
	}
	
	// メッシュごとにindexリソースを作る
	for (auto& mesh : newModel.meshes) {
		mesh.indexResource = directXBasic_->CreateBufferResource(sizeof(uint32_t) * mesh.vertices.size());
		//indexバッファビューを作る
		//リソースの先頭のアドレスから使う
		mesh.indexBufferView.BufferLocation = mesh.indexResource->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		mesh.indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * mesh.vertices.size());
		//インデックスはuint32_tとする
		mesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		//plane用インデックスリソースにデータを書き込む
		uint32_t* indexData = nullptr;
		mesh.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		for (uint32_t j = 0; j < mesh.vertices.size(); j++) {
			indexData[j] = j;
		}
	}	

	// メッシュごとに使うテクスチャ番号を記録
	for (auto& mesh : newModel.meshes) {
		//テクスチャファイルが読み込まれていなかったら読み込む
		textureManager_->LoadTexture(mesh.material.textureFilePath);
		//mesh.material.textureIndex = textureManager_->GetTextureIndexByFilePath(mesh.material.textureFilePath);
	}

	modelDatas.push_back(newModel);
}

uint32_t ModelManager::GetModelIndexByFilePath(const std::string& filePath)
{
	auto it = std::find_if(
		modelDatas.begin(),
		modelDatas.end(),
		[&](ModelData& modelData) {return modelData.filePath == filePath; }
	);
	if (it != modelDatas.end()) {
		uint32_t modelIndex = static_cast<uint32_t>(std::distance(modelDatas.begin(), it));
		return modelIndex;
	}

	assert(0);
	return 0;
}


ModelData ModelManager::LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData;	//構築するModelData
	std::vector<Vector4> positions;	//位置
	std::vector<Vector3> normals;	//法線
	std::vector<Vector2> texcoords;	//テクスチャ座標
	std::string line;	//ファイルから読んだ1行を格納するもの
	int32_t meshCount = 0;
	//0番目のメッシュデータを追加
	modelData.meshes.push_back(Mesh{});

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
			modelData.meshes[meshCount].vertices.push_back(triangle[2]);
			modelData.meshes[meshCount].vertices.push_back(triangle[1]);
			modelData.meshes[meshCount].vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			//modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
			//マテリアルファイル名を保存しておく
			modelData.mtlFileName = materialFilename;
		} else if (identifier == "o") {
			if (modelData.meshes[meshCount].vertices.size() != 0) {
				meshCount++;
				//空のメッシュデータを追加
				modelData.meshes.push_back(Mesh{});
			}
		} else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.meshes[meshCount].material = LoadMaterialTemplateFile(directoryPath, modelData.mtlFileName, materialName);
		}

	}
	return modelData;

}


MaterialData ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName) {
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
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
			return materialData;
		}
	}
	//マテリアルデータが見つからない
	assert(readMtl);
	return materialData;
}