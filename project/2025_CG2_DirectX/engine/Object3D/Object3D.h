#pragma once
#include "Object3DBasic.h"
#include "../Model/ModelManager.h"
#include "../../../TransformationMatrix.h"
#include "../../../Transform.h"
#include "../Camera/Camera.h"

class Object3D
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="object3DBasic">3Dオブジェクトの基盤</param>
	/// <param name="modelManager">モデルマネージャー</param>
	/// <param name="modelFilePath">モデルのファイルパス</param>
	void Initialize(Object3DBasic* object3DBasic, ModelManager* modelManager, std::string modelFilePath);

	/// <summary>
	///	更新
	/// </summary>
	void Update();

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

	/// <summary>
	///	モデルデータのセット
	/// </summary>
	void SetModelData(std::string modelFilePath);

	/* --------- ゲッター --------- */

	/// <summary>
	///	トランスフォームのゲッター
	/// </summary>
	const struct Transform& GetTransform() const { return transform_; }


	/* --------- セッター --------- */

	/// <summary>
	///	座標のセッター
	/// </summary>
	/// <param name="translate">移動</param>
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	/// <summary>
	///	回転のセッター
	/// </summary>
	/// <param name="rotate">回転</param>
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

	/// <summary>
	///	スケールのセッター
	/// </summary>
	/// <param name="scale">スケール</param>
	void SetScale(const Vector3& scale) { transform_.scale = scale; }

	/// <summary>
	///	カメラのセッター
	/// </summary>
	/// <param name="camera">カメラ</param>
	void SetCamera(Camera* camera) { camera_ = camera; }

private:

	/* --------- private関数 --------- */

	/// <summary>
	///	WVPリソースを作成
	/// </summary>
	void CreateWVPResource();

private:

	/* --------- private変数 --------- */

	// 3Dオブジェクト基盤のポインタ
	Object3DBasic* object3DBasic_ = nullptr;
	// モデルマネージャのポインタ
	ModelManager* modelManager_ = nullptr;

	// WVPリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	// WVPデータ書き込み用
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// 描画するモデルのポインタ
	ModelData* modelData_ = nullptr;

	// トランスフォーム
	struct Transform transform_ { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	
	// 表示フラグ
	bool isDraw_ = true;

	// カメラ
	Camera* camera_ = nullptr;
};

