#pragma once
#include <cstdint>
#include <Windows.h>

class WindowsApi
{
public:	// 静的メンバ関数

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public: // 定数

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;// 横
	static const int32_t kClientHeight = 720;// 縦

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// Windowsのメッセージ処理
	/// </summary>
	bool ProcessMessage();

	/* ----- ゲッター ----- */

	/// <summary>
	/// ウィンドウハンドルのゲッター
	/// </summary>
	HWND GetHwnd() const { return hwnd; }

	/// <summary>
	/// hInstanceのゲッター
	/// </summary>
	HINSTANCE GetHInstance() const { return wndClass.hInstance; }


private:

	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wndClass{};


};

