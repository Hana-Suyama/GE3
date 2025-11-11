#include "TextureManager.h"

void TextureManager::Initialize(DirectXBasic* directXBasic, SRVManager* srvManager)
{
	directXBasic_ = directXBasic;

	srvManager_ = srvManager;

	textureDatas_.reserve(srvManager_->kMaxSRV_);
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	if (textureDatas_.contains(filePath)) {
		return;
	}

	assert(srvManager_->AllocateRimitChack(static_cast<uint32_t>(textureDatas_.size())));

	TextureData& newTex = textureDatas_[filePath];

	DirectX::ScratchImage mipImages{};
	//Textureを読んで転送する
	mipImages = directXBasic_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	newTex.textureResource_ = directXBasic_->CreateTextureResource(metadata);
	newTex.metadata_ = metadata;
	newTex.intermediateResource_ = directXBasic_->UploadTextureData(newTex.textureResource_.Get(), mipImages);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	//先頭はImGuiが使っているのでその次を使う
	newTex.srvIndex_ = srvManager_->Allocate();

	newTex.textureSrvHandleCPU_ = srvManager_->GetCPUDescriptorHandle(newTex.srvIndex_);
	newTex.textureSrvHandleGPU_ = srvManager_->GetGPUDescriptorHandle(newTex.srvIndex_);
	//SRVの生成
	directXBasic_->GetDevice()->CreateShaderResourceView(newTex.textureResource_.Get(), &srvDesc, newTex.textureSrvHandleCPU_);

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
	assert(srvManager_->AllocateRimitChack(static_cast<uint32_t>(textureDatas_.size())));

	TextureData& textureData = textureDatas_.at(filePath);

	return textureData.textureSrvHandleGPU_;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	assert(srvManager_->AllocateRimitChack(static_cast<uint32_t>(textureDatas_.size())));

	TextureData& textureData = textureDatas_.at(filePath);
	return textureData.metadata_;
}
