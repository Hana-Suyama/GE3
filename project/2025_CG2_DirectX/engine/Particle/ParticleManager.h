#pragma once
#include "../Camera/Camera.h"
#include "../TextureManager.h"
#include "Particle.h"
#include "../../../Material.h"
#include "../../../TransformationMatrix.h"
#include <random>

struct Particle {
	struct Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct Emitter {
	struct Transform transform;
	uint32_t count;
	float frequency;
	float frequencyTime;
};

struct AccelerationField {
	Vector3 acceleration;
	MyMath::AABB area;
};

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
	void Update(Vector3 EmitPos, std::mt19937& randomEngine);

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

	/// <summary>
	///	PSOの作成
	/// </summary>
	void CreatePSO();

	void CreateVertexResource();

	std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine);

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	bool IsCollision(const MyMath::AABB& aabb, const Vector3& point);

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

	static const uint32_t kNumMaxInstance = 100;
	uint32_t numInstance = 0;
	// インスタンシング用リソース
	Comptr<ID3D12Resource> instancingResource_ = nullptr;

	std::list<Particle> particles;
	ParticleForGPU* instancingData = nullptr;
	uint32_t srvIndex_ = 0;

	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	// インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	
	// マテリアル
	Material* materialData_ = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource_ = nullptr;

	const float kDeltaTime = 1.0f / 60.0f;

	Emitter emitter{};
	AccelerationField accelerationField;
};

