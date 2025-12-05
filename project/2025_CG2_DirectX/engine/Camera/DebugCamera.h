#pragma once
#include "../utility/Math/MyMath.h"

#define DIRECTINPUT_VERSION     0x0800
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class DebugCamera
{
public:
	void Initialize(const int32_t clientWidth, const int32_t clientHeight);

	void Update(const BYTE key[256]);

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }

	Matrix4x4 GetProjectionMatrix() { return projectionMatrix_; }

private:
	//累積回転行列
	Matrix4x4 matRot_;
	//ローカル座標
	Vector3 translation_ = { 0, 0, 0 };
	Vector3 targetTranslation_ = { 0, 0, 0 };
	//ビュー行列
	Matrix4x4 viewMatrix_;
	//射影行列
	Matrix4x4 projectionMatrix_;

	//カメラ速度
	const float speed_ = 0.3f;

	const Vector3 kOffset_{ 0.0f, 0.0f, -10.0f };
};

