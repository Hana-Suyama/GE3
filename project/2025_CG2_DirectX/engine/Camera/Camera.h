#pragma once
#include "../utility/Math/Matrix4x4.h"
#include "../../../Transform.h"

class Camera
{
public:

	/* --------- public関数 --------- */

	/// <summary>
	///	コンストラクタ
	/// </summary>
	Camera();

	/// <summary>
	///	更新
	/// </summary>
	void Update();

	/* --------- ゲッター --------- */

	/// <summary>
	///	ワールド行列のゲッター
	/// </summary>
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }

	/// <summary>
	///	ビュー行列のゲッター
	/// </summary>
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }

	/// <summary>
	///	プロジェクション行列のゲッター
	/// </summary>
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

	/// <summary>
	///	ビュープロジェクション行列のゲッター
	/// </summary>
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	/// <summary>
	///	回転のゲッター
	/// </summary>
	const Vector3& GetRotate() const { return transform_.rotate; }

	/// <summary>
	///	移動のゲッター
	/// </summary>
	const Vector3& GetTranslate() const { return transform_.translate; }

	/* --------- セッター --------- */

	/// <summary>
	///	移動のセッター
	/// </summary>
	/// <param name="translate">移動</param>
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	/// <summary>
	///	回転のセッター
	/// </summary>
	/// <param name="rotate">回転</param>
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	
	/// <summary>
	///	水平方向視野角のセッター
	/// </summary>
	/// <param name="fovY">水平方向視野角</param>
	void SetFovY(const float& fovY) { fovY_ = fovY; }

	/// <summary>
	///	アスペクト比のセッター
	/// </summary>
	/// <param name="aspectRatio">アスペクト比</param>
	void SetAspectRatio(const float& aspectRatio) { aspectRatio_ = aspectRatio; }

	/// <summary>
	///	ニアクリップ距離のセッター
	/// </summary>
	/// <param name="nearClip">ニアクリップ距離</param>
	void SetNearClip(const float& nearClip) { nearClip_ = nearClip; }

	/// <summary>
	///	ファークリップ距離のセッター
	/// </summary>
	/// <param name="farClip">ファークリップ距離</param>
	void SetFarClip(const float& farClip) { farClip_ = farClip; }

private:

	// トランスフォーム
	struct Transform transform_;

	// ワールド行列
	Matrix4x4 worldMatrix_;

	// ビュー行列
	Matrix4x4 viewMatrix_;

	// プロジェクション行列
	Matrix4x4 projectionMatrix_;

	// ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix_;

	// 水平方向視野角
	float fovY_;

	// アスペクト比
	float aspectRatio_;

	// ニアクリップ距離
	float nearClip_;

	// ファークリップ距離
	float farClip_;
};

