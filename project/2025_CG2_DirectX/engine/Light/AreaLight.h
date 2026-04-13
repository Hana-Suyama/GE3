#pragma once
#include <Light.h>
#include <Vector2.h>

/// <summary>
/// エリアライトクラス
/// </summary>
class AreaLight : public Light {
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 初期化(オーバーライド)
	/// </summary>
	virtual void Initialize() override {
		// 基底クラスの初期化を呼び出す
		Light::Initialize();
		// AreaLight固有の初期化
		color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
		decay_ = 2.0f;
		direction_ = Vector3{ 0.0f, -1.0f, 0.0f }.Normalize();
		radius_ = 7.0f;
		intensity_ = 4.0f;
		position_ = { 2.0f, 1.25f, 0.0f };
		size_ = Vector2{ 1.0f, 1.0f };
	}

	/// <summary>
	/// ImGuiのデバッグ描画仮想関数(オーバーライド)
	/// </summary>
	/// <param name="label">ImGuiのラベル名</param>
	void DebugDrawImGui(std::string label) override
	{
#ifdef USE_IMGUI
		if (ImGui::TreeNode((std::string("AreaLight") + label).c_str()))
		{
			Light::DebugDrawImGui(label);
			ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position_), 0.1f);
			ImGui::DragFloat("decay", &decay_, 0.1f);
			if (ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&direction_), -1.0f, 1.0f)){
				direction_ = direction_.Normalize();
			}
			ImGui::DragFloat("radius", &radius_, 0.1f);
			ImGui::DragFloat2("size", reinterpret_cast<float*>(&size_), 0.1f);
			ImGui::TreePop();
		}
#endif
	}

	/// <summary>
	/// タイプ取得関数(オーバーライド)
	/// </summary>
	/// <returns>ライトのタイプ</returns>
	LightType GetType() const override {
		return LightType::Area;
	}

	/// <summary>
	/// 位置のセッター
	/// </summary>
	/// <param name="pos">位置ベクトル</param>
	void SetPosition(const Vector3& pos) { position_ = pos; }

	/// <summary>
	/// 向きのセッター
	/// </summary>
	/// <param name="dir">向きのベクトル</param>
	void SetDirection(const Vector3& dir) {
		direction_ = dir.Normalize();
	}

	/// <summary>
	/// ライトの届く最大距離のセッター
	/// </summary>
	/// <param name="radius">最大距離</param>
	void SetRadius(float radius) { radius_ = radius; }

	/// <summary>
	/// 減衰率のセッター
	/// </summary>
	/// <param name="decay"></param>
	void SetDecay(float decay) { decay_ = decay; }

	/// <summary>
	/// サイズのセッター
	/// </summary>
	/// <param name="size"></param>
	void SetSize(Vector2 size) { size_ = size; }

	/// <summary>
	/// ライトのデータをLightData構造体に書き込む(オーバーライド)
	/// </summary>
	/// <param name="out"> </param>
	void WriteToLightData(LightData& out) const override {
		out.type = static_cast<uint32_t>(GetType());
		out.enable = enable_;
		out.color = color_;
		out.intensity = intensity_;
		out.position = position_;
		out.direction = direction_;
		out.radius = radius_;
		out.decay = decay_;
		out.size = size_;
	}

private:

	/* --------- private変数 --------- */

	// 位置
	Vector3 position_;
	// 向き
	Vector3 direction_;
	// ライトの届く最大距離
	float radius_;
	// 減衰率
	float decay_;
	// サイズ
	Vector2 size_;

};