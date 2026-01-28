#pragma once
#include <windows.h>
#define DIRECTINPUT_VERSION     0x0800	// DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>
#include "WindowsApi.h"
#include "Vector2.h"
#include <memory>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

class Input
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

private:

	/* --------- シングルトン化 --------- */

	static std::unique_ptr<Input> instance_;
	
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
public:
	Input() = default;
	~Input() = default;

public:

	/* --------- public関数 --------- */

	// シングルトンインスタンスの取得
	static Input* GetInstance();
	// シングルトンインスタンスの解放
	static void ReleaseInstance();

	// 初期化
	void Initialize(WindowsApi* winApi);
	// 更新
	void Update(float deltaTime);

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
	bool IsPadButton(WORD button);

	// ゲームパッドを押した瞬間かをチェック
	bool IsPadButtonDown(WORD button);

	// ゲームパッドを離した瞬間かをチェック
	bool IsPadButtonUp(WORD button);

	// 左スティックを取得
	Vector2 GetLeftStick();

	// 右スティックを取得
	Vector2 GetRightStick();

	// 左トリガーの取得
	float GetLeftTrigger();

	// 右トリガーの取得
	float GetRightTrigger();

	bool IsPadConnected();

	// --- 振動 ---

	void SetVibration(float left, float right);
	void PlayVibration(float left, float right, float time);
	void StopVibration();
	void UpdateVibration(float deltaTime);
	bool IsVibrating() const;

private:

	/* --------- private関数 --------- */

	static float NormalizeStick(short v);

private:

	/* --------- private変数 --------- */

	// WindowsApi
	WindowsApi* winApi_ = nullptr;

	// IDirectInput8のインターフェイス
	Comptr<IDirectInput8> directInput_ = nullptr;

	// キーボード
	Comptr<IDirectInputDevice8> keyboard_;

	// 全キーの入力状態
	BYTE key_[256] = {};
	// 前回の全キーの状態
	BYTE keyPre_[256] = {};

	XINPUT_STATE padState_{};
	XINPUT_STATE padStatePre_{};

	bool isVibrating_ = false;
	float vibrationTime_ = 0.0f;
	float vibrationTimer_ = 0.0f;

	float vibLeft_ = 0.0f;
	float vibRight_ = 0.0f;

	// デッドゾーン
	float deadZone_ = 0.2f;
};

