#pragma once
#include "Camera.h"
#include "TextureManager.h"
#include "Material.h"
#include "TransformationMatrix.h"
#include <random>
#include <unordered_map>
#include <vector>
#include <ParticleEmitter.h>

struct Particle {
	struct EulerTransform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
	ParticleEffectType effectType;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
	Vector3 scale;
	float padding;
};

struct ParticleCS {
	Vector3 translate;
	Vector3 scale;
	float lifeTime;
	Vector3 velocity;
	float currentTime;
	Vector4 color;
};

struct AccelerationField {
	Vector3 acceleration;
	MyMath::AABB area;
};

struct PerView {
	Matrix4x4 viewProjection;
	Matrix4x4 billboardMatrix;
};

struct PerFrame
{
	float time;
	float deltaTime;
};

struct EmitterSphere {
	Vector3 translate;
	float radius;
	uint32_t count;
	float frequency;
	float frequencyTime;
	uint32_t emit;
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

	/// <summary>
	/// CS用PSOの作成
	/// </summary>
	void CreateComputeState();

	/// <summary>
	/// CS用PSOの作成
	/// </summary>
	void CreateComputeStateEmit();

	/// <summary>
	/// CS用PSOの作成
	/// </summary>
	void CreateComputeStateUpdate();

	void CreateVertexResource(uint32_t vertexCount);

	std::list<Particle> Emit(const ParticleEmitter& emitter, std::mt19937& randomEngine);

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate, ParticleEffectType effectType);

	bool IsCollision(const MyMath::AABB& aabb, const Vector3& point);

	void DispatchInitializeParticle();

	void DispatchEmitParticle();

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

	Camera* camera_ = nullptr;

	uint32_t interval_ = 5;

	static const uint32_t kNumMaxInstance_ = 1000;
	uint32_t numInstance_ = 0;
	// インスタンシング用リソース
	Comptr<ID3D12Resource> instancingResource_ = nullptr;

	std::list<Particle> particles_;
	ParticleForGPU* instancingData_ = nullptr;
	uint32_t srvIndex_ = 0;

	// マテリアル
	Material* materialData_ = nullptr;
	// マテリアルリソース
	Comptr<ID3D12Resource> materialResource_ = nullptr;

	std::vector<ParticleEmitter> emitters_;
	std::unordered_map<ParticleEffectType, ParticleSpawnSettings> spawnSettings_;
	AccelerationField accelerationField_;

	bool isBillboard_ = false;

	struct EulerTransform uvTransform_ { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 4.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 1.0f;

	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;

	uint32_t vertexCount_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> UAVResource_;

	D3D12_CPU_DESCRIPTOR_HANDLE CSVertexUavHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE CSVertexUavHandleGPU_;

	uint32_t particleUavIndex_ = 0;
	uint32_t particleSrvIndex_ = 0;
	D3D12_RESOURCE_STATES particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	// Computeパイプラインステート
	Comptr<ID3D12PipelineState> computePipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;

	Comptr<ID3D12Resource> perViewResource_ = nullptr;
	PerView* perViewData_ = nullptr;

	Comptr<ID3D12Resource> emitterSphereResource_ = nullptr;
	EmitterSphere* emitterSphereData_ = nullptr;

	Comptr<ID3D12PipelineState> computePipelineStateEmit_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignatureEmit_;

	Comptr<ID3D12PipelineState> computePipelineStateUpdate_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignatureUpdate_;

	Comptr<ID3D12Resource> perFrameResource_ = nullptr;
	PerFrame* perFrameData_ = nullptr;

	Comptr<ID3D12Resource> freeCounterResource_ = nullptr;
	uint32_t freeCounterUavIndex_ = 0;
	D3D12_RESOURCE_STATES freeCounterResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	Comptr<ID3D12Resource> freeListResource_ = nullptr;
	uint32_t freeListUavIndex_ = 0;
	D3D12_RESOURCE_STATES freeListResourceState_ = D3D12_RESOURCE_STATE_COMMON;

	const uint32_t kMaxParticles = 1024;
};

