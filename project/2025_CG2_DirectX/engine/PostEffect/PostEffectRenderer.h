#pragma once
#include <PostEffectMaterial.h>
#include <PostEffectController.h>
#include <DirectXBasic.h>
#include <TextureManager.h>
#include <Camera.h>

class PostEffectRenderer {
public:

	void Initialize(DirectXBasic* directXbasic, SRVManager* srvManager, TextureManager* textureManager);

	void Draw(const PostEffectSettings& settings, const Camera& camera, float deltaTime);

	void DebugDraw(PostEffectSettings& settings);

private:

	DirectXBasic* directXBasic_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	uint32_t renderTextureSrvIndex_;
	uint32_t depthTextureSrvIndex_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	PostEffectMaterial* materialData_ = nullptr;

	float time_ = 0.0f;

};

