#include "TextureManager.h"

void TextureManager::Initialize(DirectXBasic* directXBasic)
{
	directXBasic_ = directXBasic;

	textureDatas.reserve(directXBasic_->kMaxSRV);
}

void TextureManager::LoadTexture(const std::string& filePath)
{

	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		return;
	}

	assert(textureDatas.size() + kSRVIndexTop < directXBasic_->kMaxSRV);

	TextureData newTex;

	DirectX::ScratchImage mipImages{};
	//Textureを読んで転送する
	newTex.filePath = filePath;
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
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size()) + kSRVIndexTop;

	newTex.textureSrvHandleCPU = directXBasic_->GetCPUDescriptorHandle(directXBasic_->GetSRVDescHeap().Get(), directXBasic_->GetSRVHeapSize(), srvIndex);
	newTex.textureSrvHandleGPU = directXBasic_->GetGPUDescriptorHandle(directXBasic_->GetSRVDescHeap().Get(), directXBasic_->GetSRVHeapSize(), srvIndex);
	//SRVの生成
	directXBasic_->GetDevice()->CreateShaderResourceView(newTex.textureResource.Get(), &srvDesc, newTex.textureSrvHandleCPU);

	textureDatas.push_back(newTex);

}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	assert(textureIndex + kSRVIndexTop < static_cast<uint32_t>(directXBasic_->kMaxSRV));

	TextureData& textureData = textureDatas.at(textureIndex);

	return textureData.textureSrvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	assert(textureIndex + kSRVIndexTop < static_cast<uint32_t>(directXBasic_->kMaxSRV));

	TextureData& textureData = textureDatas.at(textureIndex);
	return textureData.metadata;
}
