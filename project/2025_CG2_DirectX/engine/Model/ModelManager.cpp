#include "ModelManager.h"
#include "Material.h"
#include <numbers>
#include <Transform.h>
#include <algorithm>

void ModelManager::Initialize(DirectXBasic* directXBasic, TextureManager* textureManager, SRVManager* srvManager)
{
	directXBasic_ = directXBasic;

	textureManager_ = textureManager;

	srvManager_ = srvManager;
}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename)
{
	// 同じモデルが読み込まれていたら読み込まない
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath_ == directoryPath + "/" + filename; }
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
	newModel.filePath_ = directoryPath + "/" + filename;

	// メッシュごとに頂点リソースを作る
	for (auto& mesh : newModel.meshes_) {
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

		uint32_t vertexSrvIndex = srvManager_->Allocate();
		mesh.vertexSrvHandle.first = srvManager_->GetCPUDescriptorHandle(vertexSrvIndex);
		mesh.vertexSrvHandle.second = srvManager_->GetGPUDescriptorHandle(vertexSrvIndex);
		srvManager_->CreateSRVforStructuredBuffer(
			vertexSrvIndex,
			mesh.vertexResource.Get(),
			static_cast<UINT>(mesh.vertices.size()),
			sizeof(VertexData));
	}

	// メッシュごとにデフォルトマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	// 現在はファイルから読み込んでおらず全部同じ値なので意味はない
	for (auto& mesh : newModel.meshes_) {
		mesh.defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialData = nullptr;
		//書き込むためのアドレスを取得
		mesh.defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//今回は白を書き込んでみる
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialData->enableLighting = Reflectance::HalfLambert;
		//UVTransformを単位行列で初期化
		materialData->uvTransform = Matrix4x4::MakeIdentity4x4();

		materialData->shininess = 1.0f;
	}
	
	// メッシュごとにindexリソースを作る
	for (auto& mesh : newModel.meshes_) {
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
	for (auto& mesh : newModel.meshes_) {
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
		[&](Model& modelData) {return modelData.filePath_ == directoryPath + "/" + filename; }
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
	newModel.filePath_ = directoryPath + "/" + filename;

	// メッシュごとに頂点リソースを作る
	for (auto& mesh : newModel.meshes_) {
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

		uint32_t vertexSrvIndex = srvManager_->Allocate();
		mesh.vertexSrvHandle.first = srvManager_->GetCPUDescriptorHandle(vertexSrvIndex);
		mesh.vertexSrvHandle.second = srvManager_->GetGPUDescriptorHandle(vertexSrvIndex);
		srvManager_->CreateSRVforStructuredBuffer(
			vertexSrvIndex,
			mesh.vertexResource.Get(),
			static_cast<UINT>(mesh.vertices.size()),
			sizeof(VertexData));
	}

	// メッシュごとにデフォルトマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	// 現在はファイルから読み込んでおらず全部同じ値なので意味はない
	for (auto& mesh : newModel.meshes_) {
		mesh.defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialData = nullptr;
		//書き込むためのアドレスを取得
		mesh.defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//今回は白を書き込んでみる
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//Lighting有効
		materialData->enableLighting = Reflectance::HalfLambert;
		//UVTransformを単位行列で初期化
		materialData->uvTransform = Matrix4x4::MakeIdentity4x4();

		materialData->shininess = 1.0f;
	}

	// メッシュごとにindexリソースを作る
	for (auto& mesh : newModel.meshes_) {
		mesh.indexResource = directXBasic_->CreateBufferResource(sizeof(uint32_t) * mesh.indices_.size());
		//indexバッファビューを作る
		//リソースの先頭のアドレスから使う
		mesh.indexBufferView.BufferLocation = mesh.indexResource->GetGPUVirtualAddress();
		//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
		mesh.indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * mesh.indices_.size());
		//インデックスはuint32_tとする
		mesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		//インデックスリソースにデータを書き込む
		uint32_t* indexData = nullptr;
		mesh.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		std::memcpy(indexData, mesh.indices_.data(), sizeof(uint32_t) * mesh.indices_.size());
	}

	// メッシュごとに使うテクスチャ番号を記録
	for (auto& mesh : newModel.meshes_) {
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

void ModelManager::CreateSphere()
{

	// すでに球が構築されていたら構築しない
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath_ == "debug_sphere"; }
	);
	if (it != modelDatas_.end()) {
		return;
	}

	// モデル上限を超えて読み込もうとしたら止める
	assert(modelDatas_.size() < kModelMax_);

	// 新しく追加する空のモデルデータを作成
	Model newModel;
	newModel.meshes_.push_back(Model::Mesh{});

	//ファイルパスを記録
	newModel.filePath_ = "debug_sphere";

	const uint32_t kSubdivision = 16;//分割数
	const float kLonEvery = std::numbers::pi_v<float> * 2.0f / float(kSubdivision);//経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / float(kSubdivision);//緯度分割1つ分の角度

	//球を構成する頂点の数
	int32_t vertexTotalNumber = kSubdivision * kSubdivision * 6;

	float space = 1.0f / kSubdivision;

	//緯度の方向に分割 0 ~ 2π
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = (-std::numbers::pi_v<float>) / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//現在の経度

			VertexData a, b, c, d;

			a.falseUV = false;
			b.falseUV = false;
			c.falseUV = false;
			d.falseUV = false;

			//頂点にデータを入力する。基準点a
			//a(左下)
			a.position.x = cosf(lat) * cosf(lon);
			a.position.y = sinf(lat);
			a.position.z = cosf(lat) * sinf(lon);
			a.position.w = 1.0f;
			a.texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };
			a.normal.x = a.position.x;
			a.normal.y = a.position.y;
			a.normal.z = a.position.z;

			//b(左上)
			b.position.x = cosf(lat + kLatEvery) * cosf(lon);
			b.position.y = sinf(lat + kLatEvery);
			b.position.z = cosf(lat + kLatEvery) * sinf(lon);
			b.position.w = 1.0f;
			b.texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) - space };
			b.normal.x = b.position.x;
			b.normal.y = b.position.y;
			b.normal.z = b.position.z;

			//c(右下)
			c.position.x = cosf(lat) * cosf(lon + kLonEvery);
			c.position.y = sinf(lat);
			c.position.z = cosf(lat) * sinf(lon + kLonEvery);
			c.position.w = 1.0f;
			c.texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) };
			c.normal.x = c.position.x;
			c.normal.y = c.position.y;
			c.normal.z = c.position.z;

			//d(右上)
			d.position.x = cosf(lat + kLatEvery) * cosf(lon + kLonEvery);
			d.position.y = sinf(lat + kLatEvery);
			d.position.z = cosf(lat + kLatEvery) * sinf(lon + kLonEvery);
			d.position.w = 1.0f;
			d.texcoord = { float(lonIndex) / float(kSubdivision) + space, 1.0f - float(latIndex) / float(kSubdivision) - space };
			d.normal.x = d.position.x;
			d.normal.y = d.position.y;
			d.normal.z = d.position.z;

			newModel.meshes_.at(0).vertices.push_back(a);
			newModel.meshes_.at(0).vertices.push_back(b);
			newModel.meshes_.at(0).vertices.push_back(c);

			newModel.meshes_.at(0).vertices.push_back(c);
			newModel.meshes_.at(0).vertices.push_back(b);
			newModel.meshes_.at(0).vertices.push_back(d);
		}
	}

	//球の頂点リソース
	newModel.meshes_.at(0).vertexResource = directXBasic_->CreateBufferResource(sizeof(VertexData) * vertexTotalNumber);
	//リソースの先頭のアドレスから使う
	newModel.meshes_.at(0).vertexBufferView.BufferLocation = newModel.meshes_.at(0).vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは分割数×分割数×6のサイズ
	newModel.meshes_.at(0).vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexTotalNumber;
	//1頂点当たりのサイズ
	newModel.meshes_.at(0).vertexBufferView.StrideInBytes = sizeof(VertexData);
	////球の頂点リソースにデータを書き込む
	VertexData* vertexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	newModel.meshes_.at(0).vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
	std::memcpy(vertexDataSphere, newModel.meshes_.at(0).vertices.data(), sizeof(VertexData)* newModel.meshes_.at(0).vertices.size());

	//球用のマテリアルリソースを作る
	newModel.meshes_.at(0).defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	Material* materialDataSphere = nullptr;
	//書き込むためのアドレスを取得
	newModel.meshes_.at(0).defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//白
	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialDataSphere->enableLighting = Reflectance::HalfLambert;
	//UVTransformを単位行列で初期化
	materialDataSphere->uvTransform = Matrix4x4::MakeIdentity4x4();

	materialDataSphere->shininess = 1.0f;

	//球用のindexリソース
	newModel.meshes_.at(0).indexResource = directXBasic_->CreateBufferResource(sizeof(uint32_t) * vertexTotalNumber);
	//リソースの先頭のアドレスから使う
	newModel.meshes_.at(0).indexBufferView.BufferLocation = newModel.meshes_.at(0).indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	newModel.meshes_.at(0).indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * vertexTotalNumber);
	//インデックスはuint32_tとする
	newModel.meshes_.at(0).indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//球用インデックスリソースにデータを書き込む
	uint32_t* indexDataSphere = nullptr;
	newModel.meshes_.at(0).indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = float(-std::numbers::pi_v<float>) / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//現在の経度
			indexDataSphere[start] = start;
			indexDataSphere[start + 1] = start + 1;
			indexDataSphere[start + 2] = start + 2;
			indexDataSphere[start + 3] = start + 1;
			indexDataSphere[start + 4] = start + 5;
			indexDataSphere[start + 5] = start + 2;
		}
	}

	// メッシュごとに使うテクスチャ番号を記録
	// テクスチャが存在しない場合、white1x1を読み込み割り当てる
	textureManager_->LoadTexture("resources/monsterBall.png");
	newModel.meshes_.at(0).defaultTextureFilePath = "resources/monsterBall.png";

	newModel.rootNode_.localMatrix = Matrix4x4::MakeIdentity4x4();

	modelDatas_.push_back(newModel);
}

void ModelManager::CreateSkyBox()
{
	// すでにスカイボックスが構築されていたら構築しない
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath_ == "debug_skybox"; }
	);
	if (it != modelDatas_.end()) {
		return;
	}

	// モデル上限を超えて読み込もうとしたら止める
	assert(modelDatas_.size() < kModelMax_);

	// 新しく追加する空のモデルデータを作成
	Model newModel;
	newModel.meshes_.push_back(Model::Mesh{});

	//ファイルパスを記録
	newModel.filePath_ = "debug_skybox";
		
	VertexData vertexData[24];

	// 右面。描画インデックスは[0,1,2][2,1,3]で内側を向く
	vertexData[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// 左面。描画インデックスは[4,5,6][6,5,7]
	vertexData[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };

	// 前面。描画インデックスは[8,9,10][10,9,11]
	vertexData[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };

	// 後面。描画インデックスは[12,13,14][14,13,15]
	vertexData[12].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[13].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[14].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[15].position = { -1.0f, -1.0f, -1.0f, 1.0f };

	// 上面。描画インデックスは[16,17,18][18,17,19]
	vertexData[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[17].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[18].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 下面。描画インデックスは[20,21,22][22,21,23]
	vertexData[20].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[21].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	for (int i = 0; i < 24; i++) {
		vertexData[i].texcoord = { 0.0f, 0.0f };
		vertexData[i].normal = { 0.0f, 0.0f, 0.0f };
		vertexData[i].falseUV = false;
	}


	// 頂点データをモデルの頂点配列にコピーする

	for (int i = 0; i < 6; i++) {
		newModel.meshes_.at(0).vertices.push_back(vertexData[0 + 4 * i]);
		newModel.meshes_.at(0).vertices.push_back(vertexData[1 + 4 * i]);
		newModel.meshes_.at(0).vertices.push_back(vertexData[2 + 4 * i]);

		newModel.meshes_.at(0).vertices.push_back(vertexData[2 + 4 * i]);
		newModel.meshes_.at(0).vertices.push_back(vertexData[1 + 4 * i]);
		newModel.meshes_.at(0).vertices.push_back(vertexData[3 + 4 * i]);
	}

	//球の頂点リソース
	newModel.meshes_.at(0).vertexResource = directXBasic_->CreateBufferResource(sizeof(VertexData) * newModel.meshes_.at(0).vertices.size());
	//リソースの先頭のアドレスから使う
	newModel.meshes_.at(0).vertexBufferView.BufferLocation = newModel.meshes_.at(0).vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは分割数×分割数×6のサイズ
	newModel.meshes_.at(0).vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * newModel.meshes_.at(0).vertices.size());
	//1頂点当たりのサイズ
	newModel.meshes_.at(0).vertexBufferView.StrideInBytes = sizeof(VertexData);
	////球の頂点リソースにデータを書き込む
	VertexData* vertexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	newModel.meshes_.at(0).vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
	std::memcpy(vertexDataSphere, newModel.meshes_.at(0).vertices.data(), sizeof(VertexData) * newModel.meshes_.at(0).vertices.size());

	//球用のマテリアルリソースを作る
	newModel.meshes_.at(0).defaultMaterialResource = directXBasic_->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	Material* materialDataSphere = nullptr;
	//書き込むためのアドレスを取得
	newModel.meshes_.at(0).defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//白
	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialDataSphere->enableLighting = Reflectance::HalfLambert;
	//UVTransformを単位行列で初期化
	materialDataSphere->uvTransform = Matrix4x4::MakeIdentity4x4();

	materialDataSphere->shininess = 1.0f;

	//球用のindexリソース
	newModel.meshes_.at(0).indexResource = directXBasic_->CreateBufferResource(sizeof(uint32_t) * newModel.meshes_.at(0).vertices.size());
	//リソースの先頭のアドレスから使う
	newModel.meshes_.at(0).indexBufferView.BufferLocation = newModel.meshes_.at(0).indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス*vertexTotalNumberのサイズ
	newModel.meshes_.at(0).indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * newModel.meshes_.at(0).vertices.size());
	//インデックスはuint32_tとする
	newModel.meshes_.at(0).indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//球用インデックスリソースにデータを書き込む
	uint32_t* indexDataSkyBox = nullptr;
	newModel.meshes_.at(0).indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSkyBox));
	
	for (int i = 0; i < newModel.meshes_.at(0).vertices.size(); i++) {
		indexDataSkyBox[i] = i;
	}

	// メッシュごとに使うテクスチャ番号を記録
	// テクスチャが存在しない場合、white1x1を読み込み割り当てる
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");
	newModel.meshes_.at(0).defaultTextureFilePath = "resources/rostock_laage_airport_4k.dds";

	newModel.rootNode_.localMatrix = Matrix4x4::MakeIdentity4x4();

	modelDatas_.push_back(newModel);
}

uint32_t ModelManager::GetModelIndexByFilePath(const std::string& filePath)
{
	auto it = std::find_if(
		modelDatas_.begin(),
		modelDatas_.end(),
		[&](Model& modelData) {return modelData.filePath_ == filePath; }
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

	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する関数を利用
	QuaternionTransform transform;
	transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
	transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // x軸を反転、さらに回転方向が逆なので軸を反転させる
	transform.translate = { -translate.x, translate.y, translate.z }; // x軸を反転
	result.transform = transform;
	result.localMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	result.name = node->mName.C_Str();	// Node名を格納
	result.children.resize(node->mNumChildren);	// 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

Model::SkinCluster ModelManager::CreateSkinCluster(const Skeleton& skeleton, const Model::Mesh& modelData)
{
	Model::SkinCluster skinCluster;

	// palette用のResourceを確保
	skinCluster.paletteResource = directXBasic_->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette, skeleton.joints.size() };// spanを使ってアクセスするようにする
	uint32_t paletteSrvIndex = srvManager_->Allocate();
	skinCluster.paletteSrvHandle.first = srvManager_->GetCPUDescriptorHandle(paletteSrvIndex);
	skinCluster.paletteSrvHandle.second = srvManager_->GetGPUDescriptorHandle(paletteSrvIndex);

	// palette用のsrvを作成。StructuredBufferでアクセスできるようにする。
	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
	paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	paletteSrvDesc.Buffer.FirstElement = 0;
	paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
	paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	directXBasic_->GetDevice()->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);

	// influence用のResourceを確保。頂点ごとにinfluence情報を追加できるようにする
	skinCluster.influenceResource = directXBasic_->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());//0埋め。weightを0にしておく。
	skinCluster.mappedInfluence = { mappedInfluence, modelData.vertices.size() };
	
	// Influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	uint32_t influenceSrvIndex = srvManager_->Allocate();
	skinCluster.influenceSrvHandle.first = srvManager_->GetCPUDescriptorHandle(influenceSrvIndex);
	skinCluster.influenceSrvHandle.second = srvManager_->GetGPUDescriptorHandle(influenceSrvIndex);
	srvManager_->CreateSRVforStructuredBuffer(
		influenceSrvIndex,
		skinCluster.influenceResource.Get(),
		static_cast<UINT>(modelData.vertices.size()),
		sizeof(VertexInfluence));

	// InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), Matrix4x4::MakeIdentity4x4);

	for (const auto& jointWeight : modelData.skinClusterData) {// ModelのSkinClusterの情報を解析
		auto it = skeleton.jointMap.find(jointWeight.first);// jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
		if (it == skeleton.jointMap.end()) {// そんな名前のJointは存在しない。なので次に回す
			continue;
		}
		// (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];// 該当のvertexindexのinfluence情報を参照しておく
			for (uint32_t index = 0; index < kNumMaxInfluences; ++index) {// 空いてるところに入れる
				if (currentInfluence.weights[index] == 0.0f) {// weight==0が空いている状態なので、その場所にweightとjointのindexを代入
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}
	}

	// UAV用のResourceを確保
	skinCluster.skinnedVertexResource = directXBasic_->CreateBufferResourceUAV(sizeof(VertexData) * modelData.vertices.size());
	skinCluster.skinnedVertexBufferView.BufferLocation = skinCluster.skinnedVertexResource->GetGPUVirtualAddress();
	skinCluster.skinnedVertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.vertices.size());
	skinCluster.skinnedVertexBufferView.StrideInBytes = sizeof(VertexData);

	skinCluster.skinnedVertexUavIndex = srvManager_->Allocate();
	skinCluster.skinnedVertexUavHandleCPU = srvManager_->GetCPUDescriptorHandle(skinCluster.skinnedVertexUavIndex);
	skinCluster.skinnedVertexUavHandleGPU = srvManager_->GetGPUDescriptorHandle(skinCluster.skinnedVertexUavIndex);

	// UAVを生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = static_cast<UINT>(modelData.vertices.size());
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(VertexData);

	// 第二引数は今はnullptrにしておく
	directXBasic_->GetDevice()->CreateUnorderedAccessView(
		skinCluster.skinnedVertexResource.Get(), nullptr, &uavDesc, skinCluster.skinnedVertexUavHandleCPU);

	// スキニング対象の頂点数をCompute Shaderに渡す
	constexpr size_t kConstantBufferAlignment = 256;
	const size_t skinningInformationSize =
		(sizeof(SkinningInformation) + kConstantBufferAlignment - 1) &
		~(kConstantBufferAlignment - 1);
	skinCluster.skinningInformationResource =
		directXBasic_->CreateBufferResource(skinningInformationSize);
	skinCluster.skinningInformationResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skinCluster.mappedSkinningInformation));
	skinCluster.mappedSkinningInformation->numVertices =
		static_cast<uint32_t>(modelData.vertices.size());

	return skinCluster;
}

Model ModelManager::LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	Model modelData;	//構築するModelData
	std::vector<Vector4> positions;	//位置
	std::vector<Vector3> normals;	//法線
	std::vector<Vector2> texcoords;	//テクスチャ座標
	std::string line;	//ファイルから読んだ1行を格納するもの
	int32_t meshCount = 0;
	//0番目のメッシュデータを追加
	modelData.meshes_.push_back(Model::Mesh{});

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
			modelData.meshes_[meshCount].vertices.push_back(triangle[2]);
			modelData.meshes_[meshCount].vertices.push_back(triangle[1]);
			modelData.meshes_[meshCount].vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			//modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
			//マテリアルファイル名を保存しておく
			modelData.mtlFileName_ = materialFilename;
		} else if (identifier == "o") {
			if (modelData.meshes_[meshCount].vertices.size() != 0) {
				meshCount++;
				//空のメッシュデータを追加
				modelData.meshes_.push_back(Model::Mesh{});
			}
		} else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.meshes_[meshCount].defaultTextureFilePath = LoadMaterialTemplateFile(directoryPath, modelData.mtlFileName_, materialName);
		}

	}

	modelData.rootNode_.localMatrix = Matrix4x4::MakeIdentity4x4();

	return modelData;

}

Model ModelManager::LoadObjFileAssimp(const std::string& directoryPath, const std::string& filename)
{

	Model modelData;	//構築するModelData

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());	// 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0));	//TexcoordがないMeshは今回は非対応

		//0番目のメッシュデータを追加
		modelData.meshes_.push_back(Model::Mesh{});
		Model::Mesh& dstMesh = modelData.meshes_.back();

		dstMesh.vertices.resize(mesh->mNumVertices);// 最初に頂点数分のメモリを確保しておく

		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			// 右手系->左手系
			dstMesh.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			dstMesh.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			dstMesh.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
			dstMesh.vertices[vertexIndex].falseUV = false;
		}

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				dstMesh.indices_.push_back(vertexIndex);
			}
		}

		uint32_t materialIndex = mesh->mMaterialIndex;
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			dstMesh.defaultTextureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			Model::JointWeightData& jointWeightData = dstMesh.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MyMath::MakeAffineMatrix(
				{ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	modelData.rootNode_ = ReadNode(scene->mRootNode);

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
