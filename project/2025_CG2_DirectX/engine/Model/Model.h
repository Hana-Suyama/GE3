#pragma once
#include <string>
#include <vector>
#include "VertexData.h"
#include <wrl.h>
#include <d3d12.h>
#include <Transform.h>
#include <map>
#include <span>
#include <array>

static const uint32_t kNumMaxInfluences = 4;
struct VertexInfluence {
	std::array<float, kNumMaxInfluences> weights;
	std::array<int32_t, kNumMaxInfluences> jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;	// 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;	// 法線用
};

struct SkinningInformation {
	uint32_t numVertices;
};

class Model
{
public:

	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	struct SkinCluster {
		std::vector<Matrix4x4> inverseBindPoseMatrices;
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
		std::span<VertexInfluence> mappedInfluence;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> influenceSrvHandle;
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
		Microsoft::WRL::ComPtr<ID3D12Resource> skinningInformationResource;
		SkinningInformation* mappedSkinningInformation = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> skinnedVertexResource;
		D3D12_VERTEX_BUFFER_VIEW skinnedVertexBufferView;
		uint32_t skinnedVertexUavIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE skinnedVertexUavHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE skinnedVertexUavHandleGPU;
		D3D12_RESOURCE_STATES skinnedVertexResourceState = D3D12_RESOURCE_STATE_COMMON;
	};

public:

	struct Mesh {
		std::vector<VertexData> vertices;
		// デフォルトのテクスチャパス。モデル生成時にオブジェクト側にコピーする
		std::string defaultTextureFilePath;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> vertexSrvHandle;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// デフォルトのマテリアル。モデル生成時にオブジェクト側にコピーする
		Microsoft::WRL::ComPtr<ID3D12Resource> defaultMaterialResource;
		std::vector<uint32_t> indices_;
		std::map<std::string, JointWeightData> skinClusterData;
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
		std::span<VertexInfluence> mappedInfluence;
	};
	
	// メッシュのベクター
	std::vector<Mesh> meshes_;

	// .mtlファイルのパス
	std::string mtlFileName_;
	// モデルファイル自体のパス
	std::string filePath_;

	// ノード
	Node rootNode_;

private:

	
};

