#include "GPUParticleManager.h"
#include <Primitive.h>
#include <ImGuiManager.h>
#include <TimeManager.h>

#include <numbers>

using namespace MyMath;

void GPUParticleManager::Initialize(DirectXBasic* directXBasic, SRVManager* srvManager, Logger* logger, TextureManager* textureManager, std::string textureFilePath, Camera* camera) {
	
	// ポインタ群の記録
	directXBasic_ = directXBasic;
	srvManager_ = srvManager;
	textureManager_ = textureManager;
	textureFilePath_ = textureFilePath;
	camera_ = camera;
	logger_ = logger;

	// PSOの作成
	CreatePSO();

	// 初期化、発生、更新のComputeStateの作成
	CreateComputeStateInit();
	CreateComputeStateEmit();
	CreateComputeStateUpdate();

	// Particle1個分(Quad1つ分)の頂点リソースを作って書き込み
	CreateVertexResource(4);

	// マテリアルリソースを作って書き込み
	CreateMaterialResource();

	// PerViewリソースを作って書き込み
	CreatePerViewResource();

	// PerFrameリソースを作って書き込み
	CreatePerFrameResource();
	
	// Particle用のリソースを作って書き込み。UAV用
	CreateParticleResourceUAV();

	// カウンター用のリソースを作って書き込み。UAV用
	CreateCounterResourceUAV();

	// 空きリスト用のリソースを作って書き込み。UAV用
	CreateFreeListResourceUAV();

	// 初期化用Dispatch
	DispatchInitializeParticle();

	// emitter用リソースを作って書き込み
	CreateEmitterResource();
}

void GPUParticleManager::Update(Vector3 EmitPos) {

	// PerViewの更新
	perViewData_->viewProjection = camera_->GetViewProjectionMatrix();
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = backToFrontMatrix.Multiply(camera_->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	perViewData_->billboardMatrix = billboardMatrix;

	// PerFrameの更新
	const float deltaTime = TimeManager::GetInstance()->GetDeltaTime();
	
	perFrameData_->deltaTime = deltaTime;
	perFrameData_->time += deltaTime;

	DispatchUpdateParticle();
	DispatchEmitParticle();
}

void GPUParticleManager::Draw() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	directXBasic_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());	//PSOを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureFilePath_));
	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleSrvDescriptorIndex_));
	//Spriteの描画。変更が必要なものだけ変更する
	directXBasic_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);	//VBVを設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, perViewResource_->GetGPUVirtualAddress());

	//マテリアルCBufferの場所を設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//描画！(DrawCall/ドローコール)
	directXBasic_->GetCommandList()->DrawInstanced(vertexCount_, 1024, 0, 0);
}

void GPUParticleManager::DebugDraw() {
#ifdef USE_IMGUI
	ImGui::Begin("AllParticle");

	ImGui::End();

#endif
}

void GPUParticleManager::CreatePSO() {

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
	D3D12_ROOT_PARAMETER rootParameters[5] = {};
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
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	descriptionRootSignature.pParameters = rootParameters;	//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);	//配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//0~1の範囲外をリビート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
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
	//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/ParticleCS.VS.hlsl",
		L"vs_6_0", logger_);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/Particle.PS.hlsl",
		L"ps_6_0", logger_);
	assert(pixelShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> geometryShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/ParticleGS.hlsl",
		L"gs_6_0", logger_);
	assert(geometryShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	//graphicsPipelineStateDesc.GS = { geometryShaderBlob->GetBufferPointer(),
	//geometryShaderBlob->GetBufferSize() };//GeometryShader
	graphicsPipelineStateDesc.GS = {};//GeometryShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。点の情報だけ送るようにしたので点
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

void GPUParticleManager::CreateComputeStateInit() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/InitializeParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRanges[3]{};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0;
	descriptorRanges[0].RegisterSpace = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].RegisterSpace = 0;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[2].NumDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = 2;
	descriptorRanges[2].RegisterSpace = 0;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = directXBasic_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&computeRootSignature_));
	assert(SUCCEEDED(hr));

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = computeRootSignature_.Get();
	hr = directXBasic_->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState_));
	assert(SUCCEEDED(hr));
}

void GPUParticleManager::CreateComputeStateEmit() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/EmitParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRanges[3]{};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0;
	descriptorRanges[0].RegisterSpace = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].RegisterSpace = 0;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[2].NumDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = 2;
	descriptorRanges[2].RegisterSpace = 0;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[5]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].Descriptor.ShaderRegister = 0;
	rootParameters[3].Descriptor.RegisterSpace = 0;
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[4].Descriptor.ShaderRegister = 1;
	rootParameters[4].Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = directXBasic_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&computeRootSignatureEmit_));
	assert(SUCCEEDED(hr));

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = computeRootSignatureEmit_.Get();
	hr = directXBasic_->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineStateEmit_));
	assert(SUCCEEDED(hr));
}

void GPUParticleManager::CreateComputeStateUpdate() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/particle/UpdateParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRanges[3]{};
	for (uint32_t index = 0; index < _countof(descriptorRanges); ++index) {
		descriptorRanges[index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		descriptorRanges[index].NumDescriptors = 1;
		descriptorRanges[index].BaseShaderRegister = index;
		descriptorRanges[index].RegisterSpace = 0;
		descriptorRanges[index].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	D3D12_ROOT_PARAMETER rootParameters[4]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].Descriptor.ShaderRegister = 1;
	rootParameters[3].Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = directXBasic_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&computeRootSignatureUpdate_));
	assert(SUCCEEDED(hr));

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = computeRootSignatureUpdate_.Get();
	hr = directXBasic_->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineStateUpdate_));
	assert(SUCCEEDED(hr));
}

void GPUParticleManager::CreateVertexResource(uint32_t vertexCount) {
	//Sprite用の頂点リソースを作る
	vertexResource_ = directXBasic_->CreateBufferResource(sizeof(VertexData) * vertexCount);
	//スプライト用頂点バッファビューを作成する
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点1つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//スプライト用の頂点リソースにデータを書き込む
	ParticleMeshData meshData = Primitive::CreateQuad(1.0f, 1.0f);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, meshData.vertices.data(), sizeof(VertexData) * meshData.vertices.size());
	vertexCount_ = static_cast<uint32_t>(meshData.vertices.size());
	vertexResource_->Unmap(0, nullptr);
}

void GPUParticleManager::CreateMaterialResource() {
	//Sprite用のマテリアルリソースを作る
	materialResource_ = directXBasic_->CreateBufferResource(sizeof(Material));
	//Sprite用のマテリアルにデータを書き込む
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//白
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//SpriteはLightingしないのでfalseを設定する
	materialData_->enableLighting = Reflectance::None;
	//UVTransformを単位行列で初期化
	materialData_->uvTransform = Matrix4x4::MakeIdentity4x4();
}

void GPUParticleManager::CreatePerViewResource() {
	perViewResource_ = directXBasic_->CreateBufferResource(sizeof(PerView));
	HRESULT hr = perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	assert(SUCCEEDED(hr));
	perViewData_->viewProjection = Matrix4x4::MakeIdentity4x4();
	perViewData_->billboardMatrix = Matrix4x4::MakeIdentity4x4();
}

void GPUParticleManager::CreatePerFrameResource() {
	perFrameResource_ = directXBasic_->CreateBufferResource(sizeof(PerFrame));
	HRESULT hr = perFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));
	assert(SUCCEEDED(hr));
	perFrameData_->deltaTime = 0.0f;
	perFrameData_->time = 0.0f;
}

void GPUParticleManager::CreateParticleResourceUAV() {
	// UAV用のResourceを確保
	particleResource_ = directXBasic_->CreateBufferResourceUAV(sizeof(GPUParticle) * 1024);
	particleUavDescriptorIndex_ = srvManager_->Allocate();
	particleUavHandleCPU_ = srvManager_->GetCPUDescriptorHandle(particleUavDescriptorIndex_);
	particleUavHandleGPU_ = srvManager_->GetGPUDescriptorHandle(particleUavDescriptorIndex_);

	// UAVを生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 1024;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticle);

	// 第二引数は今はnullptrにしておく
	directXBasic_->GetDevice()->CreateUnorderedAccessView(
		particleResource_.Get(), nullptr, &uavDesc, particleUavHandleCPU_);

	particleSrvDescriptorIndex_ = srvManager_->Allocate();
	// SRVを生成
	srvManager_->CreateSRVforStructuredBuffer(particleSrvDescriptorIndex_, particleResource_.Get(), 1024, sizeof(GPUParticle));
}

void GPUParticleManager::CreateCounterResourceUAV() {
	freeCounterResource_ = directXBasic_->CreateBufferResourceUAV(sizeof(uint32_t));
	freeCounterUavIndex_ = srvManager_->Allocate();

	D3D12_UNORDERED_ACCESS_VIEW_DESC counterUavDesc{};
	counterUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	counterUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	counterUavDesc.Buffer.FirstElement = 0;
	counterUavDesc.Buffer.NumElements = 1;
	counterUavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	counterUavDesc.Buffer.CounterOffsetInBytes = 0;
	counterUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	directXBasic_->GetDevice()->CreateUnorderedAccessView(freeCounterResource_.Get(), nullptr, &counterUavDesc, srvManager_->GetCPUDescriptorHandle(freeCounterUavIndex_));
}

void GPUParticleManager::CreateFreeListResourceUAV() {
	freeListResource_ = directXBasic_->CreateBufferResourceUAV(sizeof(uint32_t) * kMaxParticles);
	freeListUavIndex_ = srvManager_->Allocate();

	D3D12_UNORDERED_ACCESS_VIEW_DESC freeListUavDesc{};
	freeListUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	freeListUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	freeListUavDesc.Buffer.FirstElement = 0;
	freeListUavDesc.Buffer.NumElements = kMaxParticles;
	freeListUavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	freeListUavDesc.Buffer.CounterOffsetInBytes = 0;
	freeListUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	directXBasic_->GetDevice()->CreateUnorderedAccessView(freeListResource_.Get(), nullptr, &freeListUavDesc, srvManager_->GetCPUDescriptorHandle(freeListUavIndex_));
}

void GPUParticleManager::CreateEmitterResource() {
	emitterResource_ = directXBasic_->CreateBufferResource(sizeof(GPUParticleEmitter));
	const HRESULT hr = emitterResource_->Map(0, nullptr, reinterpret_cast<void**>(&emitterData_));
	assert(SUCCEEDED(hr));

	*emitterData_ = {
		.position = { 0.0f, 0.0f, 0.0f },
		.emissionRate = 10.0f,
		.rotation = { 0.0f, 0.0f, 0.0f },
		.emissionAccumulator = 0.0f,
		.scale = { 1.0f, 1.0f, 1.0f },
		.effectIndex = 0u,
		.shapeType = static_cast<uint32_t>(ParticleShapeType::Sphere),
		.randomSeed = 1u,
		.active = 1u,
		.burstCount = 0u,
		// Sphereではxを半径として使用する
		.shapeParameter = { 1.0f, 0.0f, 0.0f, 0.0f },
	};
}

void GPUParticleManager::DispatchInitializeParticle() {
	ID3D12GraphicsCommandList* commandList =
		directXBasic_->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		srvManager_->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// COMMON → UAV
	directXBasic_->TransitionBarrier(particleResource_.Get(), particleResourceState_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	directXBasic_->TransitionBarrier(freeCounterResource_.Get(), freeCounterResourceState_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	freeCounterResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	directXBasic_->TransitionBarrier(freeListResource_.Get(), freeListResourceState_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	freeListResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	// 初期化用Compute Shader
	commandList->SetComputeRootSignature(computeRootSignature_.Get());

	commandList->SetPipelineState(computePipelineState_.Get());

	commandList->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavDescriptorIndex_));

	commandList->SetComputeRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex_));

	commandList->SetComputeRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(freeListUavIndex_));

	commandList->Dispatch(1, 1, 1);

	// UAV → Vertex Shaderから読めるSRV状態
	directXBasic_->TransitionBarrier(particleResource_.Get(), particleResourceState_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	particleResourceState_ = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	directXBasic_->UAVBarrier(freeCounterResource_.Get());

	directXBasic_->UAVBarrier(freeListResource_.Get());
}

void GPUParticleManager::DispatchUpdateParticle() {
	ID3D12GraphicsCommandList* commandList = directXBasic_->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		srvManager_->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// COMMON → UAV
	directXBasic_->TransitionBarrier(particleResource_.Get(), particleResourceState_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	particleResourceState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	commandList->SetComputeRootSignature(computeRootSignatureUpdate_.Get());

	commandList->SetPipelineState(computePipelineStateUpdate_.Get());

	commandList->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavDescriptorIndex_));

	commandList->SetComputeRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex_));

	commandList->SetComputeRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(freeListUavIndex_));

	commandList->SetComputeRootConstantBufferView(3, perFrameResource_->GetGPUVirtualAddress());

	commandList->Dispatch(1, 1, 1);

	// バリア
	directXBasic_->UAVBarrier(particleResource_.Get());
	directXBasic_->UAVBarrier(freeCounterResource_.Get());
	directXBasic_->UAVBarrier(freeListResource_.Get());
	
}

void GPUParticleManager::DispatchEmitParticle() {
	ID3D12GraphicsCommandList* commandList = directXBasic_->GetCommandList();

	// 初期化用Compute Shader
	commandList->SetComputeRootSignature(computeRootSignatureEmit_.Get());

	commandList->SetPipelineState(computePipelineStateEmit_.Get());

	commandList->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavDescriptorIndex_));

	commandList->SetComputeRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex_));

	commandList->SetComputeRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(freeListUavIndex_));

	commandList->SetComputeRootConstantBufferView(3, emitterResource_->GetGPUVirtualAddress());

	commandList->SetComputeRootConstantBufferView(4, perFrameResource_->GetGPUVirtualAddress());

	commandList->Dispatch(1, 1, 1);

	// UAV → Vertex Shaderから読めるSRV状態
	directXBasic_->TransitionBarrier(particleResource_.Get(), particleResourceState_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	particleResourceState_ = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	directXBasic_->UAVBarrier(freeCounterResource_.Get());
	directXBasic_->UAVBarrier(freeListResource_.Get());
}
