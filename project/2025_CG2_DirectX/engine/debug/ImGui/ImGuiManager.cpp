#include "ImGuiManager.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

void ImGuiManager::Initialize([[maybe_unused]]WindowsApi* winApi, [[maybe_unused]] DirectXBasic* directXBasic, [[maybe_unused]] SRVManager* srvManager)
{
#ifdef USE_IMGUI
	winApi_ = winApi;
	directXBasic_ = directXBasic;
	srvManager_ = srvManager;

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
