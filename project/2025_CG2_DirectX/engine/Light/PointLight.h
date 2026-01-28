#pragma once
#include <Light.h>

/// <summary>
/// 点光源クラス
/// </summary>
class PointLight : public Light {
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 初期化(オーバーライド)
	/// </summary>
	/// <param name="directXBasic">DirectX基盤のポインタ</param>
	virtual void Initialize() override {
		// 基底クラスの初期化を呼び出す
		Light::Initialize();
		// PointLight固有の初期化
		position_ = {};
		decay_ = 1.0f;
		radius_ = 30.0f;
	}

	/// <summary>
	/// タイプ取得関数(オーバーライド)
	/// </summary>
	/// <returns>ライトのタイプ</returns>
	LightType GetType() const override {
		return LightType::Point;
	}

	/// <summary>
	/// 位置のセッター
	/// </summary>
	/// <param name="pos">位置ベクトル</param>
	void SetPosition(const Vector3& pos) { position_ = pos; }

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
	/// ライトのデータをLightData構造体に書き込む(オーバーライド)
	/// </summary>
	/// <param name="out"> </param>
	void WriteToLightData(LightData& out) const override {
		out.type = static_cast<uint32_t>(GetType());
		out.enable = enable_;
		out.color = color_;
		out.intensity = intensity_;
		out.position = position_;
		out.radius = radius_;
		out.direction = {};
		out.decay = 0.0f;
		out.cosAngle = 0.0f;
		out.cosFalloffStart = 0.0f;
	}

private:

	/* --------- private変数 --------- */

	// 位置
	Vector3 position_;
	// 届く最大距離
	float radius_;
	// 減衰率
	float decay_;

};