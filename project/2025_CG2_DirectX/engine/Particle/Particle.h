#pragma once
#include <d3d12.h>
#include "../DirectXBasic.h"
#include "../../../Transform.h"
#include "../../../TransformationMatrix.h"
#include <wrl.h>
#include "../../../VertexData.h"
#include "../../../Material.h"

class Particle
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	void Initialize(DirectXBasic* directXBasic);

	DirectXBasic* directXBasic_ = nullptr;
	// WVPリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	// WVPデータ書き込み用
	TransformationMatrix* transformationMatrixData_ = nullptr;

	int32_t limit_ = 120;
	// トランスフォーム
	struct Transform transform_ { { 0.5f, 0.5f, 1.0f }, { 0.0f, -3.14f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

	// マテリアル
	Material* materialData_ = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource_ = nullptr;

	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	// インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	Vector3 velocity_{};
	float speed_ = 0.03f;
private:

};

