#pragma once
#include "Object3DBasic.h"
#include "ModelManager.h"
#include "TransformationMatrix.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"
#include <AnimationManager.h>

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
	///	デバッグ描画
	/// </summary>
	/// <param name="label">ImGuiのラベル名</param>
	void DebugDraw(std::string label);

	/// <summary>
	///	モデルデータのセット
	/// </summary>
	void SetModelData(std::string modelFilePath);

	/* --------- ゲッター --------- */

	/// <summary>
	///	表示フラグのゲッター
	/// </summary>
	const bool& GetIsDraw() const { return isDraw_; }

	/// <summary>
	///	トランスフォームのゲッター
	/// </summary>
	const struct EulerTransform& GetTransform() const { return transform_; }

	/// <summary>
	///	マテリアルデータのゲッター
	/// </summary>
	std::vector<Material*> GetMaterialData() const { return materialDatas_; }

	/* --------- セッター --------- */

	/// <summary>
	///	表示フラグのセッター
	/// </summary>
	/// <param name="isDraw">表示/非表示</param>
	void SetIsDraw(const bool& isDraw) { isDraw_ = isDraw; }

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

	/// <summary>
	///	現在のモデルデータに基づいてマテリアル、テクスチャ、UVトランスフォームを生成
	/// </summary>
	void CreateMTUV();

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
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> skeletonDebugTransformationMatrixResources_;
	std::vector<TransformationMatrix*> skeletonDebugTransformationMatrixDatas_;

	// 描画するモデルのポインタ
	Model* modelData_ = nullptr;
	Model* skeletonDebugSphereModelData_ = nullptr;
	// マテリアルのリソース。使用するモデルのメッシュ数と同じだけ要素を持つ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResources_;
	// マテリアルのデータ。使用するモデルのメッシュ数と同じだけ要素を持つ
	std::vector<Material*> materialDatas_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> skeletonDebugMaterialResources_;
	std::vector<Material*> skeletonDebugMaterialDatas_;

	// 使用するテクスチャ。使用するモデルのメッシュ数と同じだけ要素を持つ
	std::vector<std::string> textureFilePaths_;
	// 映り込み用のテクスチャ
	std::string cubeTextureFilePaths_;
	std::vector<std::string> skeletonDebugTextureFilePaths_;

	// トランスフォーム
	struct EulerTransform transform_ { { 1.0f, 1.0f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	// UVトランスフォーム。使用するモデルのメッシュ数と同じだけ要素を持つ
	std::vector<struct EulerTransform> uvTransforms_;
	
	// 表示フラグ
	bool isDraw_ = true;

	// カメラ
	Camera* camera_ = nullptr;

};

