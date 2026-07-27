#include "ParticleManager.h"
#include "VertexData.h"
#include "TransformationMatrix.h"
#include <algorithm>
#include <numbers>
#include "ImGuiManager.h"
#include <TimeManager.h>
#include "../Primitive/Primitive.h"

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

	CreatePSO();

	CreateComputeState();

	CreateComputeStateEmit();

	CreateComputeStateUpdate();

	CreateVertexResource(4);

	//スプライト用の頂点リソースにデータを書き込む
	ParticleMeshData meshData = Primitive::CreateQuad(1.0f, 1.0f);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, meshData.vertices.data(), sizeof(VertexData) * meshData.vertices.size());
	vertexCount_ = static_cast<uint32_t>(meshData.vertices.size());
	vertexResource_->Unmap(0, nullptr);

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

	instancingResource_ = directXBasic_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance_);
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	for (uint32_t index = 0; index < kNumMaxInstance_; ++index) {
		instancingData_[index].WVP = Matrix4x4::MakeIdentity4x4();
		instancingData_[index].World = Matrix4x4::MakeIdentity4x4();
		instancingData_[index].color = Vector4( 1.0f, 1.0f, 1.0f, 1.0f );
		instancingData_[index].scale = Vector3(1.0f, 1.0f, 1.0f);
	}

	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(srvIndex_, instancingResource_.Get(), kNumMaxInstance_, sizeof(ParticleForGPU));
	
	/*for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		particles[index] = MakeNewParticle(randomEngine);
	}*/

	perViewResource_ = directXBasic_->CreateBufferResource(sizeof(PerView));
	HRESULT hr = perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	assert(SUCCEEDED(hr));
	perViewData_->viewProjection = Matrix4x4::MakeIdentity4x4();
	perViewData_->billboardMatrix = Matrix4x4::MakeIdentity4x4();

	perFrameResource_ = directXBasic_->CreateBufferResource(sizeof(PerFrame));
	hr = perFrameResource_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));
	assert(SUCCEEDED(hr));
	perFrameData_->deltaTime = 0.0f;
	perFrameData_->time = 0.0f;

	// UAV用のResourceを確保
	UAVResource_ = directXBasic_->CreateBufferResourceUAV(sizeof(ParticleCS) * 1024);
	particleUavIndex_ = srvManager_->Allocate();
	CSVertexUavHandleCPU_ = srvManager_->GetCPUDescriptorHandle(particleUavIndex_);
	CSVertexUavHandleGPU_ = srvManager_->GetGPUDescriptorHandle(particleUavIndex_);

	// UAVを生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 1024;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(ParticleCS);

	// 第二引数は今はnullptrにしておく
	directXBasic_->GetDevice()->CreateUnorderedAccessView(
		UAVResource_.Get(), nullptr, &uavDesc, CSVertexUavHandleCPU_);

	particleSrvIndex_ = srvManager_->Allocate();
	// SRVを生成
	srvManager_->CreateSRVforStructuredBuffer(
		particleSrvIndex_,
		UAVResource_.Get(),
		1024,
		sizeof(ParticleCS));

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

	directXBasic_->GetDevice()->CreateUnorderedAccessView( freeCounterResource_.Get(), nullptr, &counterUavDesc, srvManager_->GetCPUDescriptorHandle(freeCounterUavIndex_));

	DispatchInitializeParticle();

	emitterSphereResource_ = directXBasic_->CreateBufferResource(sizeof(EmitterSphere));
	emitterSphereResource_->Map(0, nullptr, reinterpret_cast<void**>(&emitterSphereData_));
	
	emitterSphereData_->count = 10;
	emitterSphereData_->frequency = 0.5f;
	emitterSphereData_->frequencyTime = 0.0f;
	emitterSphereData_->translate = Vector3(0.0f, 0.0f, 0.0f);
	emitterSphereData_->radius = 1.0f;
	emitterSphereData_->emit = 0;

	accelerationField_.acceleration = { 0.0f, 15.0f, 0.0f };
	accelerationField_.area.min = { -1.0f, -1.0f, -1.0f };
	accelerationField_.area.max = { 1.0f, 1.0f, 1.0f };

	spawnSettings_[ParticleEffectType::Hit] = {
	.scale = { 1.0f, 1.0f, 1.0f },
	.velocityMin = { 0.0f, 0.0f, 0.0f },
	.velocityMax = { 0.0f, 0.0f, 0.0f },
	.color = { 0.0f, 0.0f, 1.0f, 1.0f },
	.lifeTime = 10.0f,
	};

	spawnSettings_[ParticleEffectType::Smoke] = {
		.scale = { 2.0f, 2.0f, 2.0f },
		.velocityMin = { -0.2f, 0.5f, -0.2f },
		.velocityMax = { 0.2f, 1.2f, 0.2f },
		.color = { 0.4f, 0.4f, 0.4f, 0.6f },
		.lifeTime = 2.0f,
	};

	spawnSettings_[ParticleEffectType::Circle] = {
		.scale = { 2.0f, 2.0f, 2.0f },
		.velocityMin = { -0.2f, 0.5f, -0.2f },
		.velocityMax = { 0.2f, 1.2f, 0.2f },
		.color = { 0.4f, 0.4f, 0.4f, 0.6f },
		.lifeTime = 2.0f,
	};

	spawnSettings_[ParticleEffectType::Cylinder] = {
		.scale = { 2.0f, 2.0f, 2.0f },
		.velocityMin = { -0.2f, 0.5f, -0.2f },
		.velocityMax = { 0.2f, 1.2f, 0.2f },
		.color = { 0.4f, 0.4f, 0.4f, 0.6f },
		.lifeTime = 2.0f,
	};
	
	/*emitters_.emplace_back();
	emitters_.back().Initialize(&particles_, this, ParticleEffectType::Hit);

	emitters_.emplace_back();
	emitters_.back().Initialize(&particles_, this, ParticleEffectType::Smoke);

	emitters_.emplace_back();
	emitters_.back().Initialize(&particles_, this, ParticleEffectType::Circle);*/

	emitters_.emplace_back();
	emitters_.back().Initialize(&particles_, this, ParticleEffectType::Smoke);
}

void ParticleManager::Update(Vector3 EmitPos, std::mt19937& randomEngine)
{

	for (auto& emitter : emitters_) {
		emitter.Update(randomEngine);
	}

#ifdef USE_IMGUI
	ImGui::Begin("AllParticle");

	ImGui::Checkbox("isBillboard", &isBillboard_);
	ImGui::End();

#endif

	/*Matrix4x4 billboardMatrix = Matrix4x4::MakeIdentity4x4();
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	billboardMatrix = backToFrontMatrix.Multiply(camera_->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;*/

	perViewData_->viewProjection = camera_->GetViewProjectionMatrix();
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = backToFrontMatrix.Multiply(camera_->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	perViewData_->billboardMatrix = billboardMatrix;

	const float deltaTime = TimeManager::GetInstance()->GetDeltaTime();

	perFrameData_->deltaTime = deltaTime;
	perFrameData_->time += deltaTime;

	//uvTransform_.translate.x += 0.001f;

	//パラメータからUVTransform用の行列を生成する
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = uvTransformMatrix.Multiply(MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = uvTransformMatrix.Multiply(MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;

	numInstance_ = 0;

	for (std::list<Particle>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end();) {

		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles_.erase(particleIterator);
			continue;
		}

		if (IsCollision(accelerationField_.area, (*particleIterator).transform.translate)) {
			(*particleIterator).velocity += accelerationField_.acceleration * TimeManager::GetInstance()->GetDeltaTime();
		}
		(*particleIterator).transform.translate += (*particleIterator).velocity * TimeManager::GetInstance()->GetDeltaTime();
		(*particleIterator).currentTime += TimeManager::GetInstance()->GetDeltaTime();

		if ((*particleIterator).effectType == ParticleEffectType::Cylinder) {
			float t = (*particleIterator).currentTime / (*particleIterator).lifeTime;
			t = std::clamp(t, 0.0f, 1.0f);

			float bounce = std::sin(t * std::numbers::pi_v<float>);
			float expand = 0.2f + t * 2.8f;

			(*particleIterator).transform.scale = {
				expand,
				0.05f + bounce * 0.45f,
				expand
			};
			(*particleIterator).color.w = 1.0f - t;
		} else {
			(*particleIterator).color = { (std::max)((*particleIterator).color.x -= 0.01f, 0.0f), (std::max)((*particleIterator).color.y -= 0.006f, 0.0f), (std::min)((*particleIterator).color.z += 0.004f, 1.0f) };
		}

		Matrix4x4 worldMatrix = MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);
		if (isBillboard_) {
			worldMatrix = MakeScaleMatrix((*particleIterator).transform.scale) * billboardMatrix * MakeTranslateMatrix((*particleIterator).transform.translate);
		}
		Matrix4x4 worldViewProjectionMatrix = worldMatrix.Multiply(camera_->GetViewProjectionMatrix());


		if (numInstance_ < kNumMaxInstance_) {
			instancingData_[numInstance_].WVP = worldViewProjectionMatrix;
			instancingData_[numInstance_].World = worldMatrix;
			instancingData_[numInstance_].color = (*particleIterator).color;

			instancingData_[numInstance_].color.w = (*particleIterator).color.w;

			++numInstance_;
		}

		++particleIterator;
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
	
	emitterSphereData_->frequencyTime += TimeManager::GetInstance()->GetDeltaTime();
	if (emitterSphereData_->frequency <= emitterSphereData_->frequencyTime) {
		emitterSphereData_->frequencyTime -= emitterSphereData_->frequency;
		emitterSphereData_->emit = 1;
	} else {
		emitterSphereData_->emit = 0;
	}
	DispatchEmitParticle();
}

void ParticleManager::Draw()
{
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXBasic_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	directXBasic_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());	//PSOを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	directXBasic_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(textureFilePath_));
	//テクスチャを指定
	directXBasic_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleSrvIndex_));
	//Spriteの描画。変更が必要なものだけ変更する
	directXBasic_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);	//VBVを設定
	directXBasic_->GetCommandList()->SetGraphicsRootConstantBufferView(4, perViewResource_->GetGPUVirtualAddress());
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
	//描画！(DrawCall/ドローコール)
	//if (numInstance_) {
		directXBasic_->GetCommandList()->DrawInstanced(vertexCount_, 1024, 0, 0);
	//}
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = directXBasic_->CompileShader(L"resources/shaders/ParticleCS.VS.hlsl",
		L"vs_6_0", logger_);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = directXBasic_->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0", logger_);
	assert(pixelShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> geometryShaderBlob = directXBasic_->CompileShader(L"resources/shaders/ParticleGS.hlsl",
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

void ParticleManager::CreateComputeState() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/InitializeParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRanges[2]{};
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

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];

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

void ParticleManager::CreateComputeStateEmit() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/EmitParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRanges[2]{};
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

	D3D12_ROOT_PARAMETER rootParameters[4]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

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

void ParticleManager::CreateComputeStateUpdate() {
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = directXBasic_->CompileShader(L"resources/shaders/UpdateParticle.CS.hlsl",
		L"cs_6_0", logger_);
	assert(computeShaderBlob != nullptr);

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 1;
	rootParameters[1].Descriptor.RegisterSpace = 0;

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

void ParticleManager::CreateVertexResource(uint32_t vertexCount) {
	//Sprite用の頂点リソースを作る
	vertexResource_ = directXBasic_->CreateBufferResource(sizeof(VertexData) * vertexCount);
	//スプライト用頂点バッファビューを作成する
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点1つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

std::list<Particle> ParticleManager::Emit(const ParticleEmitter& emitter, std::mt19937& randomEngine)
{
	std::list<Particle> particles;
	for (uint32_t count = 0; count < emitter.GetCount(); ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.GetTransform().translate, emitter.GetEffectType()));
	}
	return particles;
}

//void ParticleManager::Emit(Vector3 position)
//{
//	/*intervl_--;
//	if (intervl_ <= 0) {
//		Particle* particle = new Particle();
//		particle->Initialize(directXBasic_);
//		particle->transform_.translate = position;
//		particle->velocity_.x = (cosf(DEGtoRAD(static_cast<float>(rand() % 360))) * 1.0f);
//		particle->velocity_.y = (sinf(DEGtoRAD(static_cast<float>(rand() % 360))) * 1.0f);
//		particle->velocity_ = Normalize(particle->velocity_);
//		particles_.push_back(particle);
//		intervl_ = 5;
//	}*/
//	
//}

Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate, ParticleEffectType effectType)
{
	Particle particle{};

	switch (effectType) {
		case ParticleEffectType::Hit: {
			particle.transform.scale = { 0.05f, 1.0f, 1.0f };// 横に潰す
			particle.transform.rotate = { 0.0f, 0.0f ,0.0f };
			particle.transform.translate = translate;
			particle.velocity = { 0.0f, 0.0f, 0.0f };// 動かない
			particle.color = { 0.0f, 0.0f, 1.0f, 1.0f };
			particle.lifeTime = 1.0f;// 1秒で消える
			particle.currentTime = 0.0f;

			std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
			particle.transform.rotate.z = distRotate(randomEngine);
			break;
		}
		case ParticleEffectType::Smoke: {
			std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
			
			particle.transform.scale = { 3.0f, 3.0f, 1.0f };
			particle.transform.rotate = { 0.0f, -3.14f ,0.0f };
			Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
			particle.transform.translate = translate + randomTranslate;
			particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

			std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
			particle.color = { 0.5f, 0.2f, 0.8f, 1.0f };

			std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
			particle.lifeTime = distTime(randomEngine);
			particle.currentTime = 0;
			break;
		}
		case ParticleEffectType::Circle: {
			particle.transform.scale = { 1.0f, 1.0f, 1.0f };// 横に潰す
			particle.transform.rotate = { 0.0f, 0.0f ,0.0f };
			particle.transform.translate = translate;
			particle.velocity = { 0.0f, 0.0f, 0.0f };// 動かない
			particle.color = { 0.0f, 0.0f, 1.0f, 1.0f };
			particle.lifeTime = 1.0f;// 1秒で消える
			particle.currentTime = 0.0f;
			break;
		}
		case ParticleEffectType::Cylinder: {
			particle.transform.scale = { 1.0f, 1.0f, 1.0f };// 横に潰す
			particle.transform.rotate = { 0.0f, 0.0f ,0.0f };
			particle.transform.translate = translate;
			particle.velocity = { 0.0f, 0.0f, 0.0f };// 動かない
			particle.color = { 0.0f, 0.0f, 1.0f, 1.0f };
			particle.lifeTime = 10.0f;// 10秒で消える
			particle.currentTime = 0.0f;
			break;
		}
	}

	particle.effectType = effectType;
	return particle;
}

bool ParticleManager::IsCollision(const MyMath::AABB& aabb, const Vector3& point)
{
	Vector3 closestPoint{ std::clamp(point.x, aabb.min.x, aabb.max.x),
		std::clamp(point.y, aabb.min.y, aabb.max.y),
		std::clamp(point.z, aabb.min.z, aabb.max.z) };

	float distance = Length(closestPoint, point);

	if (distance <= 1.0f) {
		return true;
	}
	return false;
}

void ParticleManager::DispatchInitializeParticle()
{
	ID3D12GraphicsCommandList* commandList =
		directXBasic_->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		srvManager_->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(
		_countof(descriptorHeaps),
		descriptorHeaps);

	// COMMON → UAV
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = UAVResource_.Get();
	barrier.Transition.StateBefore = particleResourceState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);
	particleResourceState_ =
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	D3D12_RESOURCE_BARRIER counterBarrier{};
	counterBarrier.Type =
		D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	counterBarrier.Transition.pResource =
		freeCounterResource_.Get();
	counterBarrier.Transition.StateBefore =
		D3D12_RESOURCE_STATE_COMMON;
	counterBarrier.Transition.StateAfter =
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	counterBarrier.Transition.Subresource =
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &counterBarrier);

	freeCounterResourceState_ =
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	// 初期化用Compute Shader
	commandList->SetComputeRootSignature(
		computeRootSignature_.Get());

	commandList->SetPipelineState(
		computePipelineState_.Get());

	commandList->SetComputeRootDescriptorTable(
		0,
		srvManager_->GetGPUDescriptorHandle(particleUavIndex_));

	commandList->SetComputeRootDescriptorTable(
		1,
		srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex_));

	commandList->Dispatch(1, 1, 1);

	// UAV → Vertex Shaderから読めるSRV状態
	barrier.Transition.StateBefore =
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.StateAfter =
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	commandList->ResourceBarrier(1, &barrier);
	particleResourceState_ =
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	D3D12_RESOURCE_BARRIER counterUavBarrier{};
	counterUavBarrier.Type =
		D3D12_RESOURCE_BARRIER_TYPE_UAV;
	counterUavBarrier.UAV.pResource =
		freeCounterResource_.Get();

	commandList->ResourceBarrier(1, &counterUavBarrier);
}

void ParticleManager::DispatchEmitParticle() {
	ID3D12GraphicsCommandList* commandList =
		directXBasic_->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		srvManager_->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(
		_countof(descriptorHeaps),
		descriptorHeaps);

	// COMMON → UAV
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = UAVResource_.Get();
	barrier.Transition.StateBefore = particleResourceState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);
	particleResourceState_ =
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	// 初期化用Compute Shader
	commandList->SetComputeRootSignature(
		computeRootSignatureEmit_.Get());

	commandList->SetPipelineState(
		computePipelineStateEmit_.Get());

	commandList->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));

	commandList->SetComputeRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex_));

	commandList->SetComputeRootConstantBufferView(2, emitterSphereResource_->GetGPUVirtualAddress());

	commandList->SetComputeRootConstantBufferView(3, perFrameResource_->GetGPUVirtualAddress());

	commandList->Dispatch(1, 1, 1);

	// バリア
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.UAV.pResource = UAVResource_.Get();
	commandList->ResourceBarrier(1, &barrier);

	commandList->SetComputeRootSignature(
		computeRootSignatureUpdate_.Get());

	commandList->SetPipelineState(
		computePipelineStateUpdate_.Get());

	commandList->SetComputeRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(particleUavIndex_));

	commandList->SetComputeRootConstantBufferView(1, perFrameResource_->GetGPUVirtualAddress());

	commandList->Dispatch(1, 1, 1);

	// UAV → Vertex Shaderから読めるSRV状態
	D3D12_RESOURCE_BARRIER toSrvBarrier{};
	toSrvBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	toSrvBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	toSrvBarrier.Transition.pResource = UAVResource_.Get();
	toSrvBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	toSrvBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	toSrvBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &toSrvBarrier);

	particleResourceState_ = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	D3D12_RESOURCE_BARRIER counterUavBarrier{};
	counterUavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	counterUavBarrier.UAV.pResource = freeCounterResource_.Get();

	commandList->ResourceBarrier(1, &counterUavBarrier);
}
