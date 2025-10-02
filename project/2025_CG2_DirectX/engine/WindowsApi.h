#pragma once
#include <cstdint>
#include <Windows.h>

class WindowsApi
{
public:	//静的メンバ関数

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public: //定数

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:

	void Initialize();

	void Finalize();

	bool ProcessMessage();

	//getter
	HWND GetHwnd() const { return hwnd; }

	HINSTANCE GetHInstance() const { return wndClass.hInstance; }


private:
	//ウィンドウハンドル
	HWND hwnd = nullptr;

	//ウィンドウクラスの設定
	WNDCLASS wndClass{};


};

