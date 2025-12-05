#include "ModelManager.h"
#include "../../engine/Material.h"

void ModelManager::Initialize(DirectXBasic* directXBasic, TextureManager* textureManager)
{
	directXBasic_ = directXBasic;

	textureManager_ = textureManager;
}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename)
{
	// 同じモデルが読み込まれていたら読み込まない
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath == directoryPath + "/" + filename; }
	);
	if (it != modelDatas_.end()) {
		return;
	}

	// モデル上限を超えて読み込もうとしたら止める
	assert(modelDatas_.size() < kModelMax_);

	// 新しく追加する空のモデルデータを作成
	Model newModel;
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

	// メッシュごとにデフォルトマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	// 現在はファイルから読み込んでおらず全部同じ値なので意味はない
	for (auto& mesh : newModel.meshes) {
		mesh.defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialData = nullptr;
		//書き込むためのアドレスを取得
		mesh.defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//今回は白を書き込んでみる
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialData->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialData->uvTransform = MyMath::MakeIdentity4x4();

		materialData->shininess = 1.0f;
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
		if (mesh.defaultTextureFilePath != "") {
			textureManager_->LoadTexture(mesh.defaultTextureFilePath);
		} else {
			// テクスチャが存在しない場合、white1x1を読み込み割り当てる
			textureManager_->LoadTexture("resources/white2x2.png");
			mesh.defaultTextureFilePath = "resources/white2x2.png";
		}
	}

	modelDatas_.push_back(newModel);
}

void ModelManager::LoadModelAssimp(const std::string& directoryPath, const std::string& filename)
{
	// 同じモデルが読み込まれていたら読み込まない
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath == directoryPath + "/" + filename; }
	);
	if (it != modelDatas_.end()) {
		return;
	}

	// モデル上限を超えて読み込もうとしたら止める
	assert(modelDatas_.size() < kModelMax_);

	// 新しく追加する空のモデルデータを作成
	Model newModel;
	// Objを読み込む
	newModel = LoadObjFileAssimp(directoryPath, filename);

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

	// メッシュごとにデフォルトマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	// 現在はファイルから読み込んでおらず全部同じ値なので意味はない
	for (auto& mesh : newModel.meshes) {
		mesh.defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialData = nullptr;
		//書き込むためのアドレスを取得
		mesh.defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//今回は白を書き込んでみる
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialData->enableLighting = Light::HalfLambert;
		//UVTransformを単位行列で初期化
		materialData->uvTransform = MyMath::MakeIdentity4x4();

		materialData->shininess = 1.0f;
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
		if (mesh.defaultTextureFilePath != "") {
			textureManager_->LoadTexture(mesh.defaultTextureFilePath);
		} else {
			// テクスチャが存在しない場合、white1x1を読み込み割り当てる
			textureManager_->LoadTexture("resources/white2x2.png");
			mesh.defaultTextureFilePath = "resources/white2x2.png";
		}
	}

	modelDatas_.push_back(newModel);
}

uint32_t ModelManager::GetModelIndexByFilePath(const std::string& filePath)
{
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath == filePath; }
	);
	if (it != modelDatas_.end()) {
		uint32_t modelIndex = static_cast<uint32_t>(std::distance(modelDatas_.begin(), it));
		return modelIndex;
	}

	assert(0);
	return 0;
}

Model::Node ModelManager::ReadNode(aiNode* node)
{
	Model::Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;	// nodeのlocalMatrixを取得
	aiLocalMatrix.Transpose();	// 列ベクトル形式を行ベクトル形式に転置
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}

	result.name = node->mName.C_Str();	// Node名を格納
	result.children.resize(node->mNumChildren);	// 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}


Model ModelManager::LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	Model modelData;	//構築するModelData
	std::vector<Vector4> positions;	//位置
	std::vector<Vector3> normals;	//法線
	std::vector<Vector2> texcoords;	//テクスチャ座標
	std::string line;	//ファイルから読んだ1行を格納するもの
	int32_t meshCount = 0;
	//0番目のメッシュデータを追加
	modelData.meshes.push_back(Model::Mesh{});

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
				modelData.meshes.push_back(Model::Mesh{});
			}
		} else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.meshes[meshCount].defaultTextureFilePath = LoadMaterialTemplateFile(directoryPath, modelData.mtlFileName, materialName);
		}

	}

	modelData.rootNode.localMatrix = MyMath::MakeIdentity4x4();

	return modelData;

}

Model ModelManager::LoadObjFileAssimp(const std::string& directoryPath, const std::string& filename)
{

	Model modelData;	//構築するModelData
	//0番目のメッシュデータを追加
	modelData.meshes.push_back(Model::Mesh{});
	int32_t meshCount = 0;

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());	// 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0));	//TexcoordがないMeshは今回は非対応

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);	// 三角形のみサポート

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				vertex.falseUV = false;
				// aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.meshes[meshCount].vertices.push_back(vertex);
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.meshes[meshCount].defaultTextureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}


std::string ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& materialName) {
	std::string materialData;	//構築するMaterialData
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
			materialData = directoryPath + "/" + textureFilename;
			return materialData;
		}
	}
	//マテリアルデータが見つからない
	assert(readMtl);
	return materialData;
}
