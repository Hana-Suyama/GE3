#pragma once
#include <windows.h>
#define DIRECTINPUT_VERSION     0x0800	// DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>
#include "../WindowsApi.h"
#include "../Utility/Math/Vector2.h"
#include <memory>

// EnumDeviceに渡すデータの構造体
struct enumDeviceData
{
	LPDIRECTINPUT8 directInput;             // IDirectInput8のインターフェイス
	LPDIRECTINPUTDEVICE8* ppPadDevice;		// 使用するデバイスを格納するポインタのポインタ
};

enum PadButton
{
	PAD_A = 0,
	PAD_B = 1,
	PAD_X = 2,
	PAD_Y = 3,
	PAD_LB = 4,
	PAD_RB = 5,
	PAD_BACK = 6,
	PAD_START = 7,
};

class Input
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

private:

	/* --------- シングルトン化 --------- */

	static std::unique_ptr<Input> instance;
	
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
public:
	Input() = default;
	~Input() = default;

public:

	/* --------- 静的メンバ関数 --------- */

	//ゲームパッド
	static BOOL CALLBACK DeviceFindCallBack(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef);

public:

	/* --------- public関数 --------- */

	// シングルトンインスタンスの取得
	static Input* GetInstance();
	// シングルトンインスタンスの解放
	static void ReleaseInstance();

	// 初期化
	void Initialize(WindowsApi* winApi);
	// 更新
	void Update();

	/* --- キーボード --- */

	// キーの押下をチェック
	bool IsPushKey(BYTE keyNumber);

	// キーのトリガーをチェック
	bool IsTriggerKey(BYTE keyNumber);

	// キーを押した瞬間かをチェック
	bool IsTriggerPushKey(BYTE keyNumber);

	// キーを離した瞬間かをチェック
	bool IsTriggerReleaseKey(BYTE keyNumber);

	/* --- ゲームパッド --- */

	// ゲームパッドボタンの押下をチェック
	bool IsPadButton(int button);

	// ゲームパッドを押した瞬間かをチェック
	bool IsPadButtonDown(int button);

	// ゲームパッドを離した瞬間かをチェック
	bool IsPadButtonUp(int button);

	// 左スティックを取得
	Vector2 GetLeftStick(float deadZone);

	// 右スティックを取得
	Vector2 GetRightStick(float deadZone);

	// 左トリガーの取得
	float GetLeftTrigger();

	// 右トリガーの取得
	float GetRightTrigger();

	// デッドゾーンの適用
	float ApplyDeadZone(float v, float dead);

private:

	/* --------- private変数 --------- */

	// WindowsApi
	WindowsApi* winApi = nullptr;

	// IDirectInput8のインターフェイス
	Comptr<IDirectInput8> directInput = nullptr;

	// キーボード
	Comptr<IDirectInputDevice8> keyboard;
	// ゲームパッド
	Comptr<IDirectInputDevice8> gamepad;

	// 全キーの入力状態
	BYTE key[256] = {};
	// 前回の全キーの状態
	BYTE keyPre[256] = {};

	// ゲームパッドの入力状態
	DIJOYSTATE padKey;
	// 前フレームのゲームパッドの入力状態
	DIJOYSTATE padKeyPre;
};

