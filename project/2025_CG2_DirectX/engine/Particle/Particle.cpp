#include "Particle.h"
using namespace MyMath;

void Particle::Initialize(DirectXBasic* directXBasic)
{
	directXBasic_ = directXBasic;

	//Sprite用のTransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	transformationMatrixResource = directXBasic_->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//Sprite用のマテリアルリソースを作る
	materialResource = directXBasic_->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//白
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialData->enableLighting = Light::None;
	//UVTransformを単位行列で初期化
	materialData->uvTransform = MakeIdentity4x4();

	//Sprite用のindexリソース
	indexResource = directXBasic_->CreateBufferResource(sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//Sprite用インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;
}
