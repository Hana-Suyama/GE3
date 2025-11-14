#pragma once
#include "../../WindowsApi.h"
#include "../../DirectXBasic.h"
#include "../../SRVManager.h"

#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_dx12.h"
#include "../../externals/imgui/imgui_impl_win32.h"
#endif

class ImGuiManager
{
public:

	void Initialize(WindowsApi* winApi, DirectXBasic* directXBasic, SRVManager* srvManager);

	void Update();

	void Draw();

	void Finalize();

	void UpdateBegin();

	void UpdateEnd();

private:

	// WindowsApi
	WindowsApi* winApi_ = nullptr;

	// DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	SRVManager* srvManager_ = nullptr;

};

