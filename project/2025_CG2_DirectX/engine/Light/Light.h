#pragma once
#include <LightType.h>
#include <LightData.h>
#include <ImGuiManager.h>

/// <summary>
/// ライト基底クラス
/// </summary>
class Light {
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~Light() = default;	

	/// <summary>
	/// ライトの種類を返す純粋仮想関数
	/// </summary>
	/// <returns>ライトの種類</returns>
	virtual LightType GetType() const = 0;

	/// <summary>
	/// 初期化の仮想関数
	/// </summary>
	virtual void Initialize() {
		// ライト共通初期化処理
		color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
		intensity_ = 1.0f;
		enable_ = true;
	}

	/// <summary>
	/// 更新の仮想関数
	/// </summary>
	virtual void Update() {}

	/// <summary>
	/// ImGuiのデバッグ描画仮想関数
	/// </summary>
	/// <param name="label">ImGuiのラベル名</param>
	virtual void DebugDrawImGui(std::string label)
	{
#ifdef USE_IMGUI
		ImGui::Checkbox("Enable", &enable_);
		ImGui::ColorEdit3("Color", &color_.x);
		ImGui::DragFloat("Intensity", &intensity_, 0.01f, 0.0f);
#endif
	}

	/// <summary>
	/// ライトのデータをLightData構造体に書き込む純粋仮想関数
	/// </summary>
	/// <param name="out"> </param>
	virtual void WriteToLightData(LightData& out) const = 0;

	/// <summary>
	/// 色のセッター
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(const Vector4& color) { color_ = color; }

	/// <summary>
	/// 輝度のセッター
	/// </summary>
	/// <param name="intensity">輝度</param>
	void SetIntensity(float intensity) { intensity_ = intensity; }

	/// <summary>
	/// 有効/無効のセッター
	/// </summary>
	/// <param name="enable">有効/無効</param>
	void SetEnable(bool enable) { enable_ = enable; }

protected:

	/* --------- protected変数 --------- */

	// 色
	Vector4 color_{};
	// 輝度
	float intensity_;
	// 有効/無効
	bool enable_;
};