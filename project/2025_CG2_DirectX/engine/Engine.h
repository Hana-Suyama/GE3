#pragma once

#include "cmath"
#include <Windows.h>
#include <numbers>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "CameraForGPU.h"
#include "../application/SampleScene/SampleScene.h"
#include "StringUtility.h"
#include "SpriteBasic.h"
#include "VertexData.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Object3DBasic.h"
#include "ModelManager.h"
#include "Object3D.h"
#include "SRVManager.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"
#include <random>
#include "XAudio2Basic.h"
#include "../application/GameScene.h"
#include <cstdint>
#include <string>
#include <format>
#include <chrono>
#include <d3d12.h>
#include <cassert>
#include <dxgidebug.h>
#include <vector>
#include <wrl.h>
#include "Input.h"
#include "MyMath.h"
#include "DebugCamera.h"
#include "WindowsApi.h"
#include "DirectXBasic.h"
#include "Logger.h"
#include <dxcapi.h>
#include "SceneManager.h"
#include <PostEffectMaterial.h>

using namespace MyMath;

#pragma comment(lib, "dxguid.lib")

class Engine
{
public:

	virtual ~Engine() = default;

	virtual void Initialize();

	virtual void Update();

	virtual void Draw() = 0;

	void PreDraw();
	void BackBufferPreDraw();
	void SpritePreDraw();
	void ModelPreDraw();
	void DrawRenderTexture();
	void PostDraw();

	virtual void Finalize();

	void Run();

private:

	

protected:

	std::unique_ptr<TextureManager> textureManager_ = nullptr;
	std::unique_ptr<ModelManager> modelManager_ = nullptr;
	std::unique_ptr<XAudio2Basic> xaudio2Basic_ = nullptr;
	std::unique_ptr<Logger> logger_ = nullptr;
	std::unique_ptr<WindowsApi> winApi_ = nullptr;
	std::unique_ptr<DirectXBasic> directXBasic_ = nullptr;
	std::unique_ptr<SRVManager> srvManager_ = nullptr;
	std::unique_ptr<ImGuiManager> imguiManager_ = nullptr;
	std::unique_ptr<SpriteBasic> spriteBasic_ = nullptr;
	std::unique_ptr<Camera> defaultCamera_ = nullptr;
	std::unique_ptr<Object3DBasic> object3DBasic_ = nullptr;

	BYTE beforeKey_[256] = {};
	std::unique_ptr<DebugCamera> debugcamera_ = nullptr;

	bool useDebugcamera_ = false;

	std::mt19937 randomEngine_{ std::random_device{}() };

	std::unique_ptr<SceneManager> sceneManager_ = nullptr;

	uint32_t renderTextureSrvIndex;
	uint32_t depthTextureSrvIndex;

	Microsoft::WRL::ComPtr<ID3D12Resource> outlineMaterialResource_;
	PostEffectMaterial* outlineMaterialData_ = nullptr;

	float postEffectTime_ = 0.0f;
};



