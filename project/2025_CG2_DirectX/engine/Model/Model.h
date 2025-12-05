#pragma once
#include <string>
#include <vector>
#include "../../engine/VertexData.h"
#include <wrl.h>
#include <d3d12.h>

class Model
{
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
	};
	
	// メッシュのベクター
	std::vector<Mesh> meshes;

	// .mtlファイルのパス
	std::string mtlFileName;
	// モデルファイル自体のパス
	std::string filePath;

private:

	
};

