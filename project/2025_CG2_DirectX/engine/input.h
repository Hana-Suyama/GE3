#pragma once
#include <windows.h>
#define DIRECTINPUT_VERSION     0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>
#include "WindowsApi.h"

//EnumDeviceに渡すデータの構造体
struct enumDeviceData
{
	LPDIRECTINPUT8 directInput;             // IDirectInput8のインターフェイス
	LPDIRECTINPUTDEVICE8* ppPadDevice;		// 使用するデバイスを格納するポインタのポインタ
};

class Input
{
public: //静的メンバ関数
	//ゲームパッド
	static BOOL CALLBACK DeviceFindCallBack(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef);

public:

	// namespace省略
	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

	//初期化
	void Initialize(WindowsApi* winApi);
	//更新
	void Update();

	//キーの押下をチェック
	bool PushKey(BYTE keyNumber);

	//キーのトリガーをチェック
	bool TriggerKey(BYTE keyNumber);

	//キーを押した瞬間かをチェック
	bool TriggerKeyDown(BYTE keyNumber);

	//キーを離した瞬間かをチェック
	bool TriggerKeyUp(BYTE keyNumber);

	//ゲームパッドの右スティックを取得
	DIJOYSTATE GetPadKey() { return padKey; };

private:

	// WindowsApi
	WindowsApi* winApi = nullptr;

	//IDirectInput8のインターフェイス
	Comptr<IDirectInput8> directInput = nullptr;

	//キーボード
	Comptr<IDirectInputDevice8> keyboard;
	//ゲームパッド
	Comptr<IDirectInputDevice8> gamepad;

	BYTE key[256] = {};
	//前回の全キーの状態
	BYTE keyPre[256] = {};

	//ゲームパッドの入力状態
	DIJOYSTATE padKey;

};

