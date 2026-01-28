#pragma once
#include <Light.h>
#include <numbers>

/// <summary>
/// スポットライトクラス
/// </summary>
class SpotLight : public Light {
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 初期化(オーバーライド)
	/// </summary>
	/// <param name="directXBasic">DirectX基盤のポインタ</param>
	virtual void Initialize() override {
		// 基底クラスの初期化を呼び出す
		Light::Initialize();
		// SpotLight固有の初期化
		color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
		cosAngle_ = std::cos(std::numbers::pi_v<float> / 3.0f);
		decay_ = 2.0f;
		direction_ = Vector3{ -1.0f, -1.0f, 0.0f }.Normalize();
		radius_ = 7.0f;
		intensity_ = 4.0f;
		position_ = { 2.0f, 1.25f, 0.0f };
		cosFalloffStart_ = 0.0f;
	}

	/// <summary>
	/// タイプ取得関数(オーバーライド)
	/// </summary>
	/// <returns>ライトのタイプ</returns>
	LightType GetType() const override {
		return LightType::Spot;
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
	/// 角度の余弦のセッター
	/// </summary>
	/// <param name="cosAngle"></param>
	void SetCosAngle(float cosAngle) { cosAngle_ = cosAngle; }

	/// <summary>
	/// 減衰開始角度の余弦のセッター
	/// </summary>
	/// <param name="cosFalloffStart"></param>
	void SetCosFalloffStart(float cosFalloffStart) { cosFalloffStart_ = cosFalloffStart; }

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
		out.cosAngle = cosAngle_;
		out.cosFalloffStart = cosFalloffStart_;
		// cosAngleとcosFalloffStartが同じ値にならないようにする
		if (out.cosAngle == out.cosFalloffStart) {
			out.cosFalloffStart += 0.01f;
		}
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
	// 角度の余弦
	float cosAngle_;
	// 減衰開始角度の余弦
	float cosFalloffStart_;

};