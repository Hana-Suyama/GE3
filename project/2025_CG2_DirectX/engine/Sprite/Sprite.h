#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "VertexData.h"
#include "Material.h"
#include "TransformationMatrix.h"
#include "SpriteBasic.h"
#include "TextureManager.h"
#include "Transform.h"

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

	/// <summary>
	///	デバッグ描画
	/// </summary>
	void DebugDraw(std::string rabel);

	/* --------- ゲッター --------- */

	/// <summary>
	///	表示フラグのゲッター
	/// </summary>
	const bool& GetIsDraw() const { return isDraw_; }

	/// <summary>
	///	トランスフォームのゲッター
	/// </summary>
	const struct Transform& GetTransform() const { return transform_; }

	/// <summary>
	///	座標のゲッター
	/// </summary>
	const Vector2& GetPosition() const { return position_; }

	/// <summary>
	///	回転のゲッター
	/// </summary>
	const float& GetRotation() const { return rotation_; }

	/// <summary>
	///	サイズのゲッター
	/// </summary>
	const Vector2& GetSize() const { return size_; }

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
	///	表示フラグのセッター
	/// </summary>
	/// <param name="isDraw">表示/非表示</param>
	void SetIsDraw(const bool& isDraw) { isDraw_ = isDraw; }

	/// <summary>
	///	座標のセッター
	/// </summary>
	/// <param name="position">移動</param>
	void SetPosition(const Vector2& position) { position_ = position; }

	/// <summary>
	///	回転のセッター
	/// </summary>
	/// <param name="rotate">回転</param>
	void SetRotate(const float& rotate) { rotation_ = rotate; }

	/// <summary>
	///	サイズのセッター
	/// </summary>
	/// <param name="size">サイズ</param>
	void SetSize(const Vector2& size) { size_ = size; }

	/// <summary>
	///	UVTransform座標のセッター
	/// </summary>
	/// <param name="position">移動</param>
	void SetUVPosition(const Vector2& position) { uvPosition_ = position; }

	/// <summary>
	///	UVTransform回転のセッター
	/// </summary>
	/// <param name="rotate">回転</param>
	void SetUVRotate(const float& rotate) { uvRotation_ = rotate; }

	/// <summary>
	///	UVTransformサイズのセッター
	/// </summary>
	/// <param name="scale">サイズ</param>
	void SetUVSize(const Vector2& size) { uvSize_ = size; }

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

	// 表示フラグ
	bool isDraw_ = false;

	// 座標
	Vector2 position_{};
	// 回転
	float rotation_ = 0.0f;
	// サイズ
	Vector2 size_{};

	// UV座標
	Vector2 uvPosition_{};
	// UV回転
	float uvRotation_ = 0.0f;
	// UVサイズ
	Vector2 uvSize_{1.0f, 1.0f};

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

