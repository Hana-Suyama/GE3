#include "PostEffectRenderer.h"
#include <ImGuiManager.h>

void PostEffectRenderer::Initialize(DirectXBasic* directXBasic, SRVManager* srvManager, TextureManager* textureManager) {

	directXBasic_ = directXBasic;
	srvManager_ = srvManager;
	textureManager_ = textureManager;

	// ここでレンダーテクスチャ用SRVを作る
	renderTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRendertargetTexture(
		renderTextureSrvIndex_,
		directXBasic_->GetRenderTextureResource(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	);

	// DepthTexture用SRVを作る
	depthTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(
		depthTextureSrvIndex_,
		directXBasic_->GetDepthStencilResource()
	);

	// Dissolve用ノイズテクスチャを読み込む
	textureManager_->LoadTexture("resources/noise0.png");

	materialResource_ = directXBasic_->CreateBufferResource(sizeof(PostEffectMaterial));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

}

void PostEffectRenderer::Draw(const PostEffectSettings& settings, const Camera& camera, float deltaTime) {
	
	materialData_->projectionInverse = camera.GetProjectionMatrix().Inverse();

	time_ += deltaTime;
	materialData_->time = time_;

	materialData_->threshold = settings.threshold;

	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	directXBasic_->GetCommandList()->SetDescriptorHeaps(1, heaps);

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(directXBasic_->GetRootSignatureRenderTexture());
	directXBasic_->GetCommandList()->SetPipelineState(directXBasic_->GetPostEffectPSO(settings.type));	//PSOを設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(
		2,
		srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex_)
	);

	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(
		5,
		textureManager_->GetSrvHandleGPU("resources/noise0.png")
	);

	// 頂点3つ描画
	directXBasic_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void PostEffectRenderer::DebugDraw(PostEffectSettings& settings) {
#ifdef USE_IMGUI

	static constexpr const char* effectNames[] = {
		"None",
		"Grayscale",
		"Sepia",
		"Vignette",
		"BoxFilter3x3" ,
		"BoxFilter5x5" ,
		"GaussianBlur",
		"RadialBlur",
		"LuminanceOutline",
		"DepthOutline",
		"Random",
		"Dissolve"
	};

	int type = static_cast<int>(settings.type);

	ImGui::Begin("PostEffect");

	if (ImGui::Combo(
		"Effect",
		&type,
		effectNames,
		static_cast<int>(std::size(effectNames))))
	{
		settings.type =
			static_cast<PostEffectType>(type);
	}

	if (settings.type == PostEffectType::Dissolve) {
		ImGui::SliderFloat(
			"Threshold",
			&settings.threshold,
			0.0f,
			1.0f
		);
	}

	ImGui::End();
#endif
}
