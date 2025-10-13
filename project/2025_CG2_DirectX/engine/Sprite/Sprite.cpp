#include "Sprite.h"
#include "../../../externals/DirectXTex/DirectXTex.h"

void Sprite::Initialize(SpriteBasic* spriteBasic)
{

	spriteBasic_ = spriteBasic;

	CreateVertexResource();

	CreateVertexData();

	CreateMaterialResource();

	CreateTransformationMatrixResource();

	CreateIndexResource();

	CreateTransform();

	TextureUpload();

	CreateSRV();

}

void Sprite::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WindowsApi::kClientWidth), float(WindowsApi::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;

	//パラメータからUVTransform用の行列を生成する
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	materialData->uvTransform = uvTransformMatrix;
}

void Sprite::Draw()
{
	//Spriteは常にuvCheckerにする
	spriteBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	//Spriteの描画。変更が必要なものだけ変更する
	spriteBasic_->GetDirectXBasic()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);	//VBVを設定
	//TransformationMatrixCBufferの場所を設定
	spriteBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//マテリアルCBufferの場所を設定
	spriteBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//IBVを設定
	spriteBasic_->GetDirectXBasic()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	//描画！(DrawCall/ドローコール)
	//if (drawSprite) {
	spriteBasic_->GetDirectXBasic()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//}
}

void Sprite::CreateVertexResource()
{
	//Sprite用の頂点リソースを作る
	vertexResource = spriteBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(VertexData) * 4);
	//スプライト用頂点バッファビューを作成する
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateVertexData()
{
	//スプライト用の頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };//左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };//左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };//右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };//右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
}

void Sprite::CreateMaterialResource()
{
	//Sprite用のマテリアルリソースを作る
	materialResource = spriteBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//白
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialData->enableLighting = Light::None;
	//UVTransformを単位行列で初期化
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixResource()
{
	//Sprite用のTransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	transformationMatrixResource = spriteBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}

void Sprite::CreateIndexResource()
{
	//Sprite用のindexリソース
	indexResource = spriteBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(uint32_t) * 6);
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

void Sprite::CreateTransform()
{
	transform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

	//UVTransform変数を作る
	uvTransform = {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};
}

void Sprite::TextureUpload()
{
	//Textureを読んで転送する
	mipImages = spriteBasic_->GetDirectXBasic()->LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource = spriteBasic_->GetDirectXBasic()->CreateTextureResource(metadata);
	intermediateResource = spriteBasic_->GetDirectXBasic()->UploadTextureData(textureResource.Get(), mipImages);
}

void Sprite::CreateSRV()
{
	//metaDataを基にSRVの設定
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	//先頭はImGuiが使っているのでその次を使う
	spriteBasic_->GetDirectXBasic()->IncrementSRVCount();
	textureSrvHandleCPU = spriteBasic_->GetDirectXBasic()->GetCPUDescriptorHandle(spriteBasic_->GetDirectXBasic()->GetSRVDescHeap().Get(), spriteBasic_->GetDirectXBasic()->GetSRVHeapSize(), 1);
	textureSrvHandleGPU = spriteBasic_->GetDirectXBasic()->GetGPUDescriptorHandle(spriteBasic_->GetDirectXBasic()->GetSRVDescHeap().Get(), spriteBasic_->GetDirectXBasic()->GetSRVHeapSize(), 1);
	//SRVの生成
	spriteBasic_->GetDirectXBasic()->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
}
