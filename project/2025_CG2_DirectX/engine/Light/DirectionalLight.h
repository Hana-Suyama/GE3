#pragma once
#include <Light.h>

/// <summary>
/// 平行光源クラス
/// </summary>
class DirectionalLight : public Light {
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 初期化(オーバーライド)
	/// </summary>
	/// <param name="directXBasic">DirectX基盤のポインタ</param>
	virtual void Initialize() override {
		// 基底クラスの初期化を呼び出す
		Light::Initialize();
		// DirectionalLight固有の初期化
		// デフォルトの向きは真下
		direction_ = Vector3{ 0.0f, -1.0f, 0.0f };
	}

	/// <summary>
	/// タイプ取得関数(オーバーライド)
	/// </summary>
	/// <returns>ライトのタイプ</returns>
	LightType GetType() const override {
		return LightType::Directional;
	}

	/// <summary>
	/// 向きのセッター
	/// </summary>
	/// <param name="dir">向きのベクトル</param>
	void SetDirection(const Vector3& dir) {
		direction_ = dir.Normalize();
	}

	/// <summary>
	/// ライトのデータをLightData構造体に書き込む(オーバーライド)
	/// </summary>
	/// <param name="out"></param>
	void WriteToLightData(LightData& out) const override {
		out.type = static_cast<uint32_t>(GetType());
		out.enable = enable_;
		out.color = color_;
		out.intensity = intensity_;
		out.direction = direction_;
		out.position = {};
		out.radius = 0.0f;
		out.decay = 0.0f;
		out.cosAngle = 0.0f;
		out.cosFalloffStart = 0.0f;
		out.position = {};
	}

private:
	
	Vector3 direction_; //!< ライトの向き

};