#pragma once
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

#include <DirectXBasic.h>
#include <SRVManager.h>
#include <TextureManager.h>
#include <Camera.h>
#include <VertexData.h>
#include <Material.h>
#include <GPUParticle/GPUParticleEmitter.h>

#include <random>

struct GPUParticle {
    Vector3 position;
    float age;

    Vector3 velocity;
    float lifetime;

    Vector2 size;
    float rotation;
    float angularVelocity;

    Vector4 color;

    uint32_t effectIndex;
    uint32_t active;
    uint32_t randomSeed;
    uint32_t padding;
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

class GPUParticleManager {
public:

    /* --------- namespace省略 --------- */

    template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="directXBasic"></param>
    /// <param name="srvManager"></param>
    /// <param name="logger"></param>
    /// <param name="textureManager"></param>
    /// <param name="textureFilePath"></param>
    /// <param name="camera"></param>
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
    /// デバッグ描画
    /// </summary>
    void DebugDraw();

private:

    /// <summary>
    /// パーティクル描画用のPSOを作成
    /// </summary>
    void CreatePSO();

    /// <summary>
    /// パーティクル初期化用のComputeStateを作成
    /// </summary>
    void CreateComputeStateInit();

    /// <summary>
    /// パーティクル発生用のComputeStateを作成
    /// </summary>
    void CreateComputeStateEmit();

    /// <summary>
    /// パーティクル更新用のComputeStateを作成
    /// </summary>
    void CreateComputeStateUpdate();

    /// <summary>
    /// 頂点リソースの作成
    /// </summary>
    /// <param name="vertexCount"></param>
    void CreateVertexResource(uint32_t vertexCount);

    /// <summary>
    /// マテリアルリソースの作成
    /// </summary>
    void CreateMaterialResource();

    /// <summary>
    /// PerViewリソースの作成
    /// </summary>
    void CreatePerViewResource();

    /// <summary>
    /// PerFrameリソースの作成
    /// </summary>
    void CreatePerFrameResource();

    /// <summary>
    /// パーティクル用リソースの作成。UAV用
    /// </summary>
    void CreateParticleResourceUAV();

    /// <summary>
    /// カウンター用リソースの作成。UAV用
    /// </summary>
    void CreateCounterResourceUAV();

    /// <summary>
    /// 空きリスト用リソースの作成。UAV用
    /// </summary>
    void CreateFreeListResourceUAV();

    /// <summary>
    /// emitter用リソースの作成
    /// </summary>
    void CreateEmitterResource();

    /// <summary>
    /// 初期化Dispatch
    /// </summary>
    void DispatchInitializeParticle();

    /// <summary>
    /// 更新Dispatch
    /// </summary>
    void DispatchUpdateParticle();

    /// <summary>
    /// 発生Dispatch
    /// </summary>
    void DispatchEmitParticle();

private:

    // ポインタ群
    DirectXBasic* directXBasic_ = nullptr;
    SRVManager* srvManager_ = nullptr;
    Logger* logger_ = nullptr;
    Camera* camera_ = nullptr;
    TextureManager* textureManager_ = nullptr;
    std::string textureFilePath_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

    Comptr<ID3D12PipelineState> computePipelineState_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;

    Comptr<ID3D12PipelineState> computePipelineStateEmit_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignatureEmit_;

    Comptr<ID3D12PipelineState> computePipelineStateUpdate_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignatureUpdate_;

    // 頂点リソース
    Comptr<ID3D12Resource> vertexResource_ = nullptr;
    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

    uint32_t vertexCount_ = 0;

    // マテリアル
    Material* materialData_ = nullptr;
    // マテリアルリソース
    Comptr<ID3D12Resource> materialResource_ = nullptr;

    Comptr<ID3D12Resource> perViewResource_ = nullptr;
    PerView* perViewData_ = nullptr;

    Comptr<ID3D12Resource> perFrameResource_ = nullptr;
    PerFrame* perFrameData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;
    D3D12_CPU_DESCRIPTOR_HANDLE particleUavHandleCPU_;
    D3D12_GPU_DESCRIPTOR_HANDLE particleUavHandleGPU_;
    uint32_t particleUavDescriptorIndex_ = 0;
    uint32_t particleSrvDescriptorIndex_ = 0;
    D3D12_RESOURCE_STATES particleResourceState_ = D3D12_RESOURCE_STATE_COMMON;

    Comptr<ID3D12Resource> freeCounterResource_ = nullptr;
    uint32_t freeCounterUavIndex_ = 0;
    D3D12_RESOURCE_STATES freeCounterResourceState_ = D3D12_RESOURCE_STATE_COMMON;

    Comptr<ID3D12Resource> freeListResource_ = nullptr;
    uint32_t freeListUavIndex_ = 0;
    D3D12_RESOURCE_STATES freeListResourceState_ = D3D12_RESOURCE_STATE_COMMON;

    const uint32_t kMaxParticles = 1024;

    Comptr<ID3D12Resource> emitterResource_ = nullptr;
    GPUParticleEmitter* emitterData_ = nullptr;
};
