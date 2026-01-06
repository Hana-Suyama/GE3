#include "DebugCamera.h"
using namespace MyMath;

void DebugCamera::Initialize(const int32_t clientWidth, const int32_t clientHeight) {

	viewMatrix_ = Matrix4x4::MakeIdentity4x4();
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(clientWidth) / float(clientHeight), 0.1f, 100.0f);
	matRot_ = Matrix4x4::MakeIdentity4x4();
}

void DebugCamera::Update(const BYTE key[256]) {

	Matrix4x4 cameraMatrix = Matrix4x4::MakeIdentity4x4();
	cameraMatrix = cameraMatrix.Multiply(matRot_);
	cameraMatrix = cameraMatrix.Multiply(MakeTranslateMatrix(translation_));

	if (key[DIK_UP]) {
		//カメラ移動ベクトル
		Vector3 move = { 0, speed_, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_LEFT]) {
		Vector3 move = { -speed_, 0, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_RIGHT]) {
		Vector3 move = { speed_, 0, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_DOWN]) {
		Vector3 move = { 0, -speed_, 0 };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_W]) {
		Vector3 move = { 0, 0, speed_ };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	if (key[DIK_S]) {
		Vector3 move = { 0, 0, -speed_ };
		move = TransformNormal(move, cameraMatrix);

		targetTranslation_ += move;
	}

	//回転処理
	float Xrotate = 0.0f;
	float Yrotate = 0.0f;
	float Zrotate = 0.0f;

	if (key[DIK_A]) {
		Xrotate += -speed_;
	}

	if (key[DIK_D]) {
		Xrotate += speed_;
	}

	if (key[DIK_Q]) {
		Yrotate += -speed_;
	}

	if (key[DIK_E]) {
		Yrotate += speed_;
	}

	if (key[DIK_Z]) {
		Zrotate += -speed_;
	}

	if (key[DIK_C]) {
		Zrotate += speed_;
	}

	//追加回転分の回転行列を生成
	Matrix4x4 matRotDelta = Matrix4x4::MakeIdentity4x4();
	matRotDelta = matRotDelta.Multiply(MakeRotateXMatrix(Xrotate));
	matRotDelta = matRotDelta.Multiply(MakeRotateYMatrix(Yrotate));
	matRotDelta = matRotDelta.Multiply(MakeRotateZMatrix(Zrotate));

	//累積の回転行列を合成
	matRot_ = matRotDelta.Multiply(matRot_);

	Vector3 offset = TransformNormal(kOffset_, matRot_);

	translation_ = targetTranslation_ + offset;
	
	viewMatrix_ = cameraMatrix.Inverse();
}