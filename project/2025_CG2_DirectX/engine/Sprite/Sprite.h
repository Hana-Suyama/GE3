#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "../../../VertexData.h"
#include "../../../Material.h"
#include "../../../TransformationMatrix.h"
#include "SpriteBasic.h"
#include "../TextureManager.h"
#include "../../../Transform.h"

class Sprite
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	/* --------- public関数 --------- */

	/// <summary>
	///	初期化
	/// </summary>
	/// <param name="spriteBasic">Spriteの基盤</param>
	/// <param name="textureManager">テクスチャマネージャー</param>
	void Initialize(SpriteBasic* spriteBasic, TextureManager* textureManager, std::string textureFilePath);

	/// <summary>
	///	更新
	/// </summary>
	void Update();

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

	/* --------- ゲッター --------- */

	/// <summary>
	///	トランスフォームのゲッター
	/// </summary>
	const struct Transform& GetTransform() const { return transform_; }

	/// <summary>
	///	色のゲッター
	/// </summary>
	const Vector4& GetColor() const { return materialData_->color; }

	/// <summary>
	///	アンカーポイントのゲッター
	/// </summary>
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }

	/// <summary>
	///	左右反転のゲッター
	/// </summary>
	const bool& GetIsFlipX() const { return isFlipX_; }

	/// <summary>
	///	上下反転のゲッター
	/// </summary>
	const bool& GetIsFlipY() const { return isFlipY_; }

	/// <summary>
	///	テクスチャ左上座標のゲッター
	/// </summary>
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }

	/// <summary>
	///	テクスチャ切り出しサイズのゲッター
	/// </summary>
	const Vector2& GetTextureSize() const { return textureSize_; }

	/* --------- セッター --------- */

	/// <summary>
	///	座標のセッター
	/// </summary>
	/// <param name="spriteBasic">移動</param>
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	/// <summary>
	///	回転のセッター
	/// </summary>
	/// <param name="spriteBasic">回転</param>
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

	/// <summary>
	///	スケールのセッター
	/// </summary>
	/// <param name="spriteBasic">スケール</param>
	void SetScale(const Vector3& scale) { transform_.scale = scale; }

	/// <summary>
	///	色のセッター
	/// </summary>
	/// <param name="spriteBasic">色</param>
	void SetColor(const Vector4& color) { materialData_->color = color; };

	/// <summary>
	///	アンカーポイントのセッター
	/// </summary>
	/// <param name="anchorPoint">アンカーポイントの座標(0.0f~1.0f)</param>
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	/// <summary>
	///	左右反転のセッター
	/// </summary>
	/// <param name="isFlipX">左右反転するかどうか</param>
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }

	/// <summary>
	///	上下反転のセッター
	/// </summary>
	/// <param name="isFlipY">上下反転するかどうか</param>
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }

	/// <summary>
	///	テクスチャ左上座標のセッター
	/// </summary>
	/// <param name="leftTop">左上に指定する座標</param>
	void SetTextureLeftTop(const Vector2& leftTop) { textureLeftTop_ = leftTop; }

	/// <summary>
	///	テクスチャ切り出しサイズのセッター
	/// </summary>
	/// <param name="textureSize">切り出すサイズの指定</param>
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

	/// <summary>
	///	テクスチャパスのセッター
	/// </summary>
	/// <param name="textureFilePath">テクスチャのファイルパス</param>
	void SetTextureFilePath(std::string textureFilePath);

private:

	/* --------- private関数 --------- */

	/// <summary>
	///	頂点リソースを作成
	/// </summary>
	void CreateVertexResource();

	/// <summary>
	///	頂点データを作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	///	マテリアルリソースを作成
	/// </summary>
	void CreateMaterialResource();

	/// <summary>
	///	座標変換行列を作成
	/// </summary>
	void CreateTransformationMatrixResource();

	/// <summary>
	///	インデックスリソースを作成
	/// </summary>
	void CreateIndexResource();

	/// <summary>
	///	Transformを作成
	/// </summary>
	void CreateTransform();

	/// <summary>
	///	スケールを切り出しサイズに合わせる
	/// </summary>
	void AdjustTextureSize();

private:

	/* --------- private変数 --------- */

	// Sprite基盤のポインタ
	SpriteBasic* spriteBasic_ = nullptr;
	// テクスチャマネージャのポインタ
	TextureManager* textureManager_ = nullptr;

	// テクスチャの番号
	uint32_t textureIndex_ = 0;

	// 頂点リソース
	Comptr<ID3D12Resource> vertexResource_ = nullptr;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// Transform
	struct Transform transform_{};
	// UVTransform
	struct Transform uvTransform_{};
	// アンカーポイント(0.0f~1.0f)
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 左右反転
	bool isFlipX_ = false;
	// 上下反転
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f, 0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize_{};

	// 座標変換行列
	TransformationMatrix* transformationMatrixData_ = nullptr;
	// 座標変換行列リソース
	Comptr<ID3D12Resource> transformationMatrixResource_ = nullptr;

	// マテリアル
	Material* materialData_ = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource_ = nullptr;

	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	// インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	// テクスチャのファイルパス
	std::string textureFilePath_;

};

