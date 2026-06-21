#include "ImGuiManager.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

void ImGuiManager::Initialize([[maybe_unused]]WindowsApi* winApi, [[maybe_unused]] DirectXBasic* directXBasic, [[maybe_unused]] SRVManager* srvManager, [[maybe_unused]] uint32_t* renderTextureSrvIndex)
{
#ifdef USE_IMGUI
	winApi_ = winApi;
	directXBasic_ = directXBasic;
	srvManager_ = srvManager;
	renderTextureSrvIndex_ = renderTextureSrvIndex;

	uint32_t srvIndex = srvManager_->Allocate();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApi_->GetHwnd());
	ImGui_ImplDX12_Init(directXBasic_->GetDevice(),
		static_cast<int>(directXBasic_->GetSwapChainDescBufferCount()),
		directXBasic_->GetRtvDesc().Format,
		srvManager_->GetDescriptorHeap(),
		srvManager_->GetCPUDescriptorHandle(srvIndex),
		srvManager_->GetGPUDescriptorHandle(srvIndex));
#endif
}

void ImGuiManager::Update()
{
}

void ImGuiManager::ImGuiPreDraw()
{
	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = directXBasic_->GetSwapChain()->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	barrier_ = {};
	//今回のバリアはTransition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier_.Transition.pResource = directXBasic_->GetRenderTextureResource();
	//遷移前(現在)のResourceState
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//遷移後のResourceState
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//TransitionBarrierを張る
	directXBasic_->GetCommandList()->ResourceBarrier(1, &barrier_);

	directXBasic_->GetCommandList()->OMSetRenderTargets(1, &directXBasic_->GetRtvHandle(backBufferIndex), false, nullptr);
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	//青っぽい色。RGBAの順
	directXBasic_->GetCommandList()->ClearRenderTargetView(directXBasic_->GetRtvHandle(backBufferIndex), clearColor, 0, nullptr);
	
	directXBasic_->GetCommandList()->RSSetViewports(1, &directXBasic_->GetViewport());	//Viewportを設定
	directXBasic_->GetCommandList()->RSSetScissorRects(1, &directXBasic_->GetScissorRect());	//Scissorを設定

	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	directXBasic_->GetCommandList()->SetDescriptorHeaps(1, heaps);

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(directXBasic_->GetRootSignatureRenderTexture());
	directXBasic_->GetCommandList()->SetPipelineState(directXBasic_->GetGraphicsPipelineStateRenderTexture());	//PSOを設定
	
	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(
		2,
		srvManager_->GetGPUDescriptorHandle(*renderTextureSrvIndex_)
	);

	// 頂点3つ描画
	directXBasic_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void ImGuiManager::Draw()
{
#ifdef USE_IMGUI
	ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap() };
	directXBasic_->GetCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directXBasic_->GetCommandList());
#endif
}

void ImGuiManager::Finalize()
{
#ifdef USE_IMGUI
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::UpdateBegin()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::UpdateEnd()
{
#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
#endif
}
