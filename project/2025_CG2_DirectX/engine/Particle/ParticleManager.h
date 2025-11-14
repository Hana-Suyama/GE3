#pragma once
#include "../Camera/Camera.h"
#include "../TextureManager.h"
#include "Particle.h"
#include "../../../Material.h"
#include "../../../TransformationMatrix.h"

class ParticleManager
{
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	~ParticleManager();

	void Initialize(DirectXBasic* directXBasic, SRVManager* srvManager, Logger* logger, TextureManager* textureManager, std::string textureFilePath, Camera* camera);

	/// <summary>
	///	更新
	/// </summary>
	void Update(Vector3 EmitPos);

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

	/// <summary>
	///	PSOの作成
	/// </summary>
	void CreatePSO();

	void CreateVertexResource();

	void Emit(Vector3 position);

private:
	DirectXBasic* directXBasic_ = nullptr;

	SRVManager* srvManager_ = nullptr;

	/// <summary>
	///	ロガー
	/// </summary>
	Logger* logger_ = nullptr;

	/// <summary>
	///	ルートシグネチャ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	/// <summary>
	///	グラフィックスパイプラインステート
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	TextureManager* textureManager_;

	// 頂点リソース
	Comptr<ID3D12Resource> vertexResource_ = nullptr;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	std::string textureFilePath_;

	//std::list<Particle*> particles_;

	Camera* camera_ = nullptr;

	uint32_t intervl_ = 5;

	static const uint32_t kNumInstance = 10;
	// インスタンシング用リソース
	Comptr<ID3D12Resource> instancingResource_ = nullptr;

	struct Transform transforms[kNumInstance];
	TransformationMatrix* instancingData = nullptr;
	uint32_t srvIndex_ = 0;

	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	// インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	
	// マテリアル
	Material* materialData_ = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource_ = nullptr;


};

