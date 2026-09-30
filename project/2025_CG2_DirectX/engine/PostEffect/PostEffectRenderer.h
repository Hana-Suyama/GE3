#pragma once
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class Camera;
class DirectXBasic;
class SRVManager;
class TextureManager;
struct PostEffectMaterial;
struct PostEffectSettings;

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

