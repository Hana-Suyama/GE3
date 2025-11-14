#include "ParticleManager.h"
#include "../../../VertexData.h"
#include "../../../TransformationMatrix.h"

using namespace MyMath;

ParticleManager::~ParticleManager()
{
	////敵の解放
	//for (Particle* particle : particles_) {
	//	delete particle;
	//}
	//particles_.clear();
}

void ParticleManager::Initialize(DirectXBasic* directXBasic, SRVManager* srvManager, Logger* logger, TextureManager* textureManager, std::string textureFilePath, Camera* camera)
{
	directXBasic_ = directXBasic;

	srvManager_ = srvManager;

	textureManager_ = textureManager;

	textureFilePath_ = textureFilePath;

	camera_ = camera;

	//引数のロガーポインタを記録
	logger_ = logger;

	srand((unsigned)time(NULL));

	CreatePSO();

	//Sprite用の頂点リソースを作る
	vertexResource_ = directXBasic_->CreateBufferResource(sizeof(VertexData) * 4);
	//スプライト用頂点バッファビューを作成する
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//スプライト用の頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };//左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };//左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };//右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };//右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	//Sprite用のindexリソース
	indexResource_ = directXBasic_->CreateBufferResource(sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	//Sprite用インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	//Sprite用のマテリアルリソースを作る
	materialResource_ = directXBasic_->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//白
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialData_->enableLighting = Light::None;
	//UVTransformを単位行列で初期化
	materialData_->uvTransform = MakeIdentity4x4();


	instancingResource_ = directXBasic_->CreateBufferResource(sizeof(TransformationMatrix) * kNumInstance);
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		instancingData[index].WVP = MakeIdentity4x4();
		instancingData[index].World = MakeIdentity4x4();
	}

	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(srvIndex_, instancingResource_.Get(), kNumInstance, sizeof(TransformationMatrix));
	
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		transforms[index].scale = { 1.0f, 1.0f, 1.0f };
		transforms[index].rotate = { 0.0f, -3.14f ,0.0f };
		transforms[index].translate = { index * 0.1f, index * 0.1f, index * 0.1f };
	}
}

void ParticleManager::Update(Vector3 EmitPos)
{

	for (uint32_t index = 0; index < kNumInstance; ++index) {
		Matrix4x4 worldMatrix = MakeAffineMatrix(transforms[index].scale, transforms[index].rotate, transforms[index].translate);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, camera_->GetViewProjectionMatrix());
		instancingData[index].WVP = worldViewProjectionMatrix;
		instancingData[index].World = worldMatrix;
	}

	//Emit(EmitPos);
	//

	//for (Particle* particle : particles_) {
	//	Matrix4x4 worldMatrix = MakeAffineMatrix(particle->transform_.scale, particle->transform_.rotate, particle->transform_.translate);
	//	Matrix4x4 worldViewProjectionMatrix;
	//	if (camera_) {
	//		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
	//		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	//	} else {
	//		worldViewProjectionMatrix = worldMatrix;
	//	}

	//	particle->transformationMatrixData_->WVP = worldViewProjectionMatrix;
	//	particle->transformationMatrixData_->World = worldMatrix;

	//	particle->limit_--;
	//	particle->transform_.translate += particle->velocity_ * particle->speed_;
	//}

	////デスフラグの立った敵を削除
	//particles_.remove_if([](Particle* particle) {
	//	if (particle->limit_ < 0) {
	//		delete particle;
	//		return true;
	//	}
	//	return false;
	//	});
	
}

void ParticleManager::Draw()
{
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	directXBasic_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());	//PSOを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureFilePath_));
	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(srvIndex_));
	//Spriteの描画。変更が必要なものだけ変更する
	directXBasic_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);	//VBVを設定
	//for (Particle* particle : particles_) {
	//	//TransformationMatrixCBufferの場所を設定
	//	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(1, particle->transformationMatrixResource_->GetGPUVirtualAddress());
	//	//マテリアルCBufferの場所を設定
	//	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(0, particle->materialResource_->GetGPUVirtualAddress());
	//	//IBVを設定
	//	directXBasic_->GetCommandList()->IASetIndexBuffer(&particle->indexBufferView_);
	//	//描画！(DrawCall/ドローコール)
	//	//if (drawSprite) {
	//	directXBasic_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//	//}
	//}

	//マテリアルCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//IBVを設定
	directXBasic_->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	//描画！(DrawCall/ドローコール)
	directXBasic_->GetCommandList()->DrawIndexedInstanced(6, kNumInstance, 0, 0, 0);
}

void ParticleManager::CreatePSO()
{

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;	//0から始まる
	descriptorRange[0].NumDescriptors = 1;	//数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算

	// RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;	//レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;	//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);	//Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;	//レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters;	//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);	//配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//0~1の範囲外をリビート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのMipMapを使う
	staticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = directXBasic_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[4] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[3].SemanticName = "FALSEUV";
	inputElementDescs[3].SemanticIndex = 0;
	inputElementDescs[3].Format = DXGI_FORMAT_R32_SINT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = directXBasic_->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0", logger_);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = directXBasic_->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0", logger_);
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	hr = directXBasic_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreateVertexResource()
{
	//Sprite用の頂点リソースを作る
	vertexResource_ = directXBasic_->CreateBufferResource(sizeof(VertexData) * 4);
	//スプライト用頂点バッファビューを作成する
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::Emit(Vector3 position)
{
	/*intervl_--;
	if (intervl_ <= 0) {
		Particle* particle = new Particle();
		particle->Initialize(directXBasic_);
		particle->transform_.translate = position;
		particle->velocity_.x = (cosf(DEGtoRAD(static_cast<float>(rand() % 360))) * 1.0f);
		particle->velocity_.y = (sinf(DEGtoRAD(static_cast<float>(rand() % 360))) * 1.0f);
		particle->velocity_ = Normalize(particle->velocity_);
		particles_.push_back(particle);
		intervl_ = 5;
	}*/
	
}
