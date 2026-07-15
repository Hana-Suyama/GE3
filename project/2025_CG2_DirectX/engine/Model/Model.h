#pragma once
#include <string>
#include <vector>
#include "VertexData.h"
#include <wrl.h>
#include <d3d12.h>
#include <Transform.h>

class Model
{
public:

	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

public:

	struct Mesh {
		std::vector<VertexData> vertices;
		// デフォルトのテクスチャパス。モデル生成時にオブジェクト側にコピーする
		std::string defaultTextureFilePath;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// デフォルトのマテリアル。モデル生成時にオブジェクト側にコピーする
		Microsoft::WRL::ComPtr<ID3D12Resource> defaultMaterialResource;
		std::vector<uint32_t> indices_;
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

