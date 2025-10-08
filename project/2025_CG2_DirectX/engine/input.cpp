#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(WindowsApi* winApi){

	//仮引数のwinApiのインスタンスを記録
	this->winApi = winApi;

	HRESULT hr;

	//DirectInputの初期化
	hr = DirectInput8Create(
		winApi->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイスの生成
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(
		winApi->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//ゲームパッドデバイスの生成
	enumDeviceData deviceData;
	deviceData.directInput = directInput.Get();
	deviceData.ppPadDevice = &gamepad;
	//ゲームパッドの列挙
	hr = directInput->EnumDevices(DI8DEVTYPE_GAMEPAD, DeviceFindCallBack, &deviceData, DIEDFL_ATTACHEDONLY);
	assert(SUCCEEDED(hr));

	if (gamepad) {
		//入力データ形式のセット
		hr = gamepad->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(hr));

		//排他制御レベルのセット
		hr = gamepad->SetCooperativeLevel(
			winApi->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	}
}

void Input::Update(){

	//前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	//キーボード情報の取得開始
	keyboard->Acquire();

	//全キーの入力状態を取得する
	keyboard->GetDeviceState(sizeof(key), key);

	if (gamepad) {
		//ゲームパッド情報の取得開始
		gamepad->Acquire();

		//入力状態を取得する
		gamepad->GetDeviceState(sizeof(DIJOYSTATE), &padKey);
	}

}

bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	//そうでなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (key[keyNumber] != keyPre[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKeyDown(BYTE keyNumber)
{
	if (TriggerKey(keyNumber) && key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKeyUp(BYTE keyNumber)
{
	if (TriggerKey(keyNumber) && !key[keyNumber]) {
		return true;
	}
	return false;
}


// デバイス発見時に実行される
BOOL CALLBACK Input::DeviceFindCallBack(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef)
{
	enumDeviceData* deviceData = (enumDeviceData*)pvRef;

	//ゲームパッドデバイスの生成
	HRESULT hr = deviceData->directInput->CreateDevice(ipddi->guidInstance, deviceData->ppPadDevice, NULL);
	assert(SUCCEEDED(hr));

	return DIENUM_CONTINUE;
}
