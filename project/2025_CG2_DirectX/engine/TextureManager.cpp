#include "TextureManager.h"

void TextureManager::Initialize(DirectXBasic* directXBasic, SRVManager* srvManager)
{
	directXBasic_ = directXBasic;

	srvManager_ = srvManager;

	textureDatas.reserve(srvManager_->kMaxSRV);
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	if (textureDatas.contains(filePath)) {
		return;
	}

	assert(srvManager_->AllocateRimitChack(textureDatas.size()));

	TextureData& newTex = textureDatas[filePath];

	DirectX::ScratchImage mipImages{};
	//Textureを読んで転送する
	mipImages = directXBasic_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	newTex.textureResource = directXBasic_->CreateTextureResource(metadata);
	newTex.metadata = metadata;
	newTex.intermediateResource = directXBasic_->UploadTextureData(newTex.textureResource.Get(), mipImages);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	//先頭はImGuiが使っているのでその次を使う
	newTex.srvIndex = srvManager_->Allocate();

	newTex.textureSrvHandleCPU = srvManager_->GetCPUDescriptorHandle(newTex.srvIndex);
	newTex.textureSrvHandleGPU = srvManager_->GetGPUDescriptorHandle(newTex.srvIndex);
	//SRVの生成
	directXBasic_->GetDevice()->CreateShaderResourceView(newTex.textureResource.Get(), &srvDesc, newTex.textureSrvHandleCPU);

	//textureDatas.insert(newTex);

}

//uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
//{
//	if (textureDatas.contains(filePath)) {
//		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
//		return textureIndex;
//	}
//
//	assert(0);
//	return 0;
//}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	assert(textureIndex + kSRVIndexTop < static_cast<uint32_t>(directXBasic_->kMaxSRV));

	TextureData& textureData = textureDatas.at(filePath);

	return textureData.textureSrvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	assert(textureIndex + kSRVIndexTop < static_cast<uint32_t>(directXBasic_->kMaxSRV));

	TextureData& textureData = textureDatas.at(filePath);
	return textureData.metadata;
}
