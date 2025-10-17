#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "../../../2025_CG2_DirectX/engine/DirectXBasic.h"
#include "../../../2025_CG2_DirectX/engine/utility/Logger.h"

class SpriteBasic
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
	///	Sprite描画の共通部分のコマンド。Spriteの描画前に呼び出す
	/// </summary>
	void SpritePreDraw();

	/* --------- ゲッター --------- */

	/// <summary>
	///	DirectX基盤のゲッター
	/// </summary>
	DirectXBasic* GetDirectXBasic() const { return directXBasic_; };

private:

	/* --------- private関数 --------- */

	/// <summary>
	///	PSOの作成
	/// </summary>
	void CreatePSO();

private:

	/* --------- private変数 --------- */

	/// <summary>
	///	DirectX基盤のポインタ
	/// </summary>
	DirectXBasic* directXBasic_ = nullptr;

	/// <summary>
	///	ロガー
	/// </summary>
	Logger* logger_ = nullptr;

	/// <summary>
	///	ルートシグネチャ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	/// <summary>
	///	グラフィックスパイプラインステート
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

};

