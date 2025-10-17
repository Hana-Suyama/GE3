#include "DebugCamera.h"
using namespace MyMath;

void DebugCamera::Initialize(const int32_t clientWidth, const int32_t clientHeight) {

	viewMatrix = MakeIdentity4x4();
	projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(clientWidth) / float(clientHeight), 0.1f, 100.0f);
	matRot_ = MakeIdentity4x4();
}

void DebugCamera::Update(const BYTE key[256]) {

	Matrix4x4 cameraMatrix = MakeIdentity4x4();
	cameraMatrix = Multiply(cameraMatrix, matRot_);
	cameraMatrix = Multiply(cameraMatrix, MakeTranslateMatrix(translation_));

	if (key[DIK_UP]) {
		//カメラ移動ベクトル
		Vector3 move = { 0, speed, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_LEFT]) {
		Vector3 move = { -speed, 0, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_RIGHT]) {
		Vector3 move = { speed, 0, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_DOWN]) {
		Vector3 move = { 0, -speed, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_W]) {
		Vector3 move = { 0, 0, speed };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_S]) {
		Vector3 move = { 0, 0, -speed };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	//回転処理
	float Xrotate = 0.0f;
	float Yrotate = 0.0f;
	float Zrotate = 0.0f;

	if (key[DIK_A]) {
		Xrotate += -speed;
	}

	if (key[DIK_D]) {
		Xrotate += speed;
	}

	if (key[DIK_Q]) {
		Yrotate += -speed;
	}

	if (key[DIK_E]) {
		Yrotate += speed;
	}

	if (key[DIK_Z]) {
		Zrotate += -speed;
	}

	if (key[DIK_C]) {
		Zrotate += speed;
	}

	//追加回転分の回転行列を生成
	Matrix4x4 matRotDelta = MakeIdentity4x4();
	matRotDelta = Multiply(matRotDelta, MakeRotateXMatrix(Xrotate));
	matRotDelta = Multiply(matRotDelta, MakeRotateYMatrix(Yrotate));
	matRotDelta = Multiply(matRotDelta, MakeRotateZMatrix(Zrotate));

	//累積の回転行列を合成
	matRot_ = Multiply(matRotDelta, matRot_);

	Vector3 offset = TransformNormal(kOffset, matRot_);

	translation_ = targetTranslation_ + offset;
	
	viewMatrix = Inverse(cameraMatrix);
}