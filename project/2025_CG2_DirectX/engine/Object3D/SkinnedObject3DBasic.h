#pragma once
#include "DirectXBasic.h"
#include "Camera.h"


class SkinnedObject3DBasic
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
	///	Skinned3Dオブジェクト描画の共通部分のコマンド。Skinned3Dオブジェクトの描画前に呼び出す
	/// </summary>
	void SkinnedObject3DPreDraw();

	void SetComputeResources(D3D12_GPU_DESCRIPTOR_HANDLE palette, D3D12_GPU_DESCRIPTOR_HANDLE inputVertex, D3D12_GPU_DESCRIPTOR_HANDLE influence, D3D12_GPU_DESCRIPTOR_HANDLE outputVertex, D3D12_GPU_VIRTUAL_ADDRESS skinningInformation);

	void SkinningPreDispatch();

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

	/// <summary>
	/// CS用PSOの作成
	/// </summary>
	void CreateComputeState();

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

	// CS用ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_ = nullptr;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	// Computeパイプラインステート
	Comptr<ID3D12PipelineState> computePipelineState = nullptr;

};

