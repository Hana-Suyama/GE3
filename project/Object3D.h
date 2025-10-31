#pragma once
#include "Object3DBasic.h"
#include "ModelManager.h"
#include "TransformationMatrix.h"
#include "Transform.h"

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
	void Update(struct Transform cameraTransform);

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
	const struct Transform& GetTransform() const { return transform; }


	/* --------- セッター --------- */

	/// <summary>
	///	座標のセッター
	/// </summary>
	/// <param name="spriteBasic">移動</param>
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	/// <summary>
	///	回転のセッター
	/// </summary>
	/// <param name="spriteBasic">回転</param>
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }

	/// <summary>
	///	スケールのセッター
	/// </summary>
	/// <param name="spriteBasic">スケール</param>
	void SetScale(const Vector3& scale) { transform.scale = scale; }

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
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// WVPデータ書き込み用
	TransformationMatrix* transformationMatrixData = nullptr;

	// 描画するモデルのポインタ
	ModelData* modelData = nullptr;

	// トランスフォーム
	struct Transform transform { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	
	bool isDraw_ = true;
};

