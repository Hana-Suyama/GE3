//#include "Particle.h"
//using namespace MyMath;
//
//void Particle::Initialize(DirectXBasic* directXBasic)
//{
//	directXBasic_ = directXBasic;
//
//	//Sprite用のTransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
//	transformationMatrixResource_ = directXBasic_->CreateBufferResource(sizeof(TransformationMatrix));
//	//データを書き込む
//	//書き込むためのアドレスを取得
//	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
//	//単位行列を書き込んでおく
//	transformationMatrixData_->WVP = MakeIdentity4x4();
//	transformationMatrixData_->World = MakeIdentity4x4();
//
//	//Sprite用のマテリアルリソースを作る
//	materialResource_ = directXBasic_->CreateBufferResource(sizeof(Material));
//	//Sprite用のマテリアルにデータを書き込む
//	//書き込むためのアドレスを取得
//	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
//	//白
//	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
//	//SpriteはLightingしないのでfalseを設定する
//	materialData_->enableLighting = Light::None;
//	//UVTransformを単位行列で初期化
//	materialData_->uvTransform = MakeIdentity4x4();
//
//	//Sprite用のindexリソース
//	indexResource_ = directXBasic_->CreateBufferResource(sizeof(uint32_t) * 6);
//	//リソースの先頭のアドレスから使う
//	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
//	//使用するリソースのサイズはインデックス6つ分のサイズ
//	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
//	//インデックスはuint32_tとする
//	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
//	//Sprite用インデックスリソースにデータを書き込む
//	uint32_t* indexData = nullptr;
//	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
//	indexData[0] = 0;
//	indexData[1] = 1;
//	indexData[2] = 2;
//	indexData[3] = 1;
//	indexData[4] = 3;
//	indexData[5] = 2;
//}
