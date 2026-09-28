#pragma once

class DirectXBasic;
class SRVManager;
class WindowsApi;

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

