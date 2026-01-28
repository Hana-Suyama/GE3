#include "Input.h"
#include <cassert>
#include <algorithm>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define PAD_A XINPUT_GAMEPAD_A
#define PAD_B XINPUT_GAMEPAD_B
#define PAD_X XINPUT_GAMEPAD_X
#define PAD_Y XINPUT_GAMEPAD_Y
#define PAD_LB XINPUT_GAMEPAD_LEFT_SHOULDER
#define PAD_RB XINPUT_GAMEPAD_RIGHT_SHOULDER
#define PAD_START XINPUT_GAMEPAD_START
#define PAD_BACK XINPUT_GAMEPAD_BACK

// シングルトン化
std::unique_ptr<Input> Input::instance_ = nullptr;

Input* Input::GetInstance()
{
	if (!instance_) {
		instance_ = std::make_unique<Input>();
	}
	return instance_.get();
}

void Input::ReleaseInstance()
{
	instance_.reset();
}

void Input::Initialize(WindowsApi* winApi){

	//仮引数のwinApiのインスタンスを記録
	this->winApi_ = winApi;

	HRESULT hr;

	//DirectInputの初期化
	hr = DirectInput8Create(
		winApi->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイスの生成
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = keyboard_->SetCooperativeLevel(
		winApi->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

}

void Input::Update(float deltaTime){

	// 前回のキー入力を保存
	memcpy(keyPre_, key_, sizeof(key_));
	
	//キーボード情報の取得開始
	keyboard_->Acquire();

	//全キーの入力状態を取得する
	keyboard_->GetDeviceState(sizeof(key_), key_);

	padStatePre_ = padState_;

	DWORD result = XInputGetState(0, &padState_);
	if (result != ERROR_SUCCESS)
	{
		ZeroMemory(&padState_, sizeof(XINPUT_STATE));
	}

	UpdateVibration(deltaTime);

}

bool Input::IsPushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key_[keyNumber]) {
		return true;
	}
	//そうでなければfalseを返す
	return false;
}

bool Input::IsTriggerKey(BYTE keyNumber)
{
	if (key_[keyNumber] != keyPre_[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::IsTriggerPushKey(BYTE keyNumber)
{
	if (IsTriggerKey(keyNumber) && key_[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::IsTriggerReleaseKey(BYTE keyNumber)
{
	if (IsTriggerKey(keyNumber) && !key_[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::IsPadButton(WORD button)
{
	return (padState_.Gamepad.wButtons & button) != 0;
}

bool Input::IsPadButtonDown(WORD button)
{
	return (padState_.Gamepad.wButtons & button) &&
		!(padStatePre_.Gamepad.wButtons & button);
}

bool Input::IsPadButtonUp(WORD button)
{
	return !(padState_.Gamepad.wButtons & button) &&
		(padStatePre_.Gamepad.wButtons & button);
}

Vector2 Input::GetLeftStick()
{
	float x = NormalizeStick(padState_.Gamepad.sThumbLX);
	float y = NormalizeStick(padState_.Gamepad.sThumbLY);

	if (fabs(x) < deadZone_) x = 0;
	if (fabs(y) < deadZone_) y = 0;

	return { x, y };
}

Vector2 Input::GetRightStick()
{
	float x = NormalizeStick(padState_.Gamepad.sThumbRX);
	float y = NormalizeStick(padState_.Gamepad.sThumbRY);

	if (fabs(x) < deadZone_) x = 0;
	if (fabs(y) < deadZone_) y = 0;

	return { x, y };
}

float Input::GetLeftTrigger()
{
	float v = padState_.Gamepad.bLeftTrigger / 255.0f;
	return v < deadZone_ ? 0.0f : v;
}

float Input::GetRightTrigger()
{
	float v = padState_.Gamepad.bRightTrigger / 255.0f;
	return v < deadZone_ ? 0.0f : v;
}

float Input::NormalizeStick(short v)
{
	if (v < 0) return v / 32768.0f;
	return v / 32767.0f;
}

void Input::SetVibration(float left, float right)
{
	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed = static_cast<WORD>(left * 65535);
	vib.wRightMotorSpeed = static_cast<WORD>(right * 65535);
	XInputSetState(0, &vib);
}

void Input::PlayVibration(float left, float right, float time)
{
	vibLeft_ = std::clamp(left, 0.0f, 1.0f);
	vibRight_ = std::clamp(right, 0.0f, 1.0f);
	vibrationTime_ = time;
	vibrationTimer_ = 0.0f;
	isVibrating_ = true;

	SetVibration(vibLeft_, vibRight_);
}

void Input::StopVibration()
{
	isVibrating_ = false;
	vibrationTimer_ = 0.0f;
	vibrationTime_ = 0.0f;

	SetVibration(0.0f, 0.0f);
}

void Input::UpdateVibration(float deltaTime)
{
	if (!isVibrating_) return;

	vibrationTimer_ += deltaTime;

	if (vibrationTimer_ >= vibrationTime_)
	{
		StopVibration();
		return;
	}

	SetVibration(vibLeft_, vibRight_);
}

bool Input::IsVibrating() const
{
	return isVibrating_;
}

bool Input::IsPadConnected()
{
	return XInputGetState(0, &padState_) == ERROR_SUCCESS;
}
