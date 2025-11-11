#pragma once
#include "../DirectXBasic.h"
#include "../Camera/Camera.h"


class Object3DBasic
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="directXBasic">DirectXの基盤</param>
	/// <param name="logger">ロガー</param>
	void Initialize(DirectXBasic* directXBasic, Logger* logger);

	/// <summary>
	///	3Dオブジェクト描画の共通部分のコマンド。3Dオブジェクトの描画前に呼び出す
	/// </summary>
	void Object3DPreDraw();

	/* --------- ゲッター --------- */

	/// <summary>
	///	DirectX基盤のゲッター
	/// </summary>
	DirectXBasic* GetDirectXBasic() const { return directXBasic_; };

	/// <summary>
	///	デフォルトカメラのゲッター
	/// </summary>
	Camera* GetDefaultCamera() const { return defaultCamera_; }

	/* --------- セッター --------- */

	/// <summary>
	///	カメラのセッター
	/// </summary>
	/// <param name="camera">カメラ</param>
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }


private:

	/* --------- private関数 --------- */

	/// <summary>
	///	PSOの作成
	/// </summary>
	void CreatePSO();

private:

	/* --------- private変数 --------- */

	// DirectX基盤のポインタ
	DirectXBasic* directXBasic_ = nullptr;

	// ロガー
	Logger* logger_ = nullptr;

	// デフォルトカメラのポインタ
	Camera* defaultCamera_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

};

