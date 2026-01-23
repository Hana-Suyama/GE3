#pragma once

#include "cmath"
#include <Windows.h>
#include <numbers>
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Camera/CameraForGPU.h"
#include "../application/SampleScene/SampleScene.h"
#include "utility/StringUtility.h"
#include "Sprite/SpriteBasic.h"
#include "VertexData.h"
#include "Sprite/Sprite.h"
#include "TextureManager.h"
#include "Object3D/Object3DBasic.h"
#include "Model/ModelManager.h"
#include "Object3D/Object3D.h"
#include "SRVManager.h"
#include "Particle/ParticleManager.h"
#include "debug/ImGui/ImGuiManager.h"
#include <random>
#include "Audio/XAudio2Basic.h"
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
#include "Input/Input.h"
#include "utility/Math/MyMath.h"
#include "Camera/DebugCamera.h"
#include "WindowsApi.h"
#include "DirectXBasic.h"
#include "debug/Logger/Logger.h"
#include <dxcapi.h>
#include "Scene/SceneManager.h"

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
	void SpritePreDraw();
	void ModelPreDraw();
	void PostDraw();

	virtual void Finalize();

	void Run();

private:

	

protected:

	std::unique_ptr<TextureManager> textureManager = nullptr;
	std::unique_ptr<ModelManager> modelManager = nullptr;
	std::unique_ptr<XAudio2Basic> xaudio2Basic = nullptr;
	std::unique_ptr<Logger> logger = nullptr;
	std::unique_ptr<WindowsApi> winApi = nullptr;
	std::unique_ptr<DirectXBasic> directXBasic = nullptr;
	std::unique_ptr<SRVManager> srvManager = nullptr;
	std::unique_ptr<ImGuiManager> imguiManager = nullptr;
	std::unique_ptr<SpriteBasic> spriteBasic = nullptr;
	std::unique_ptr<Camera> defaultCamera = nullptr;
	std::unique_ptr<Object3DBasic> object3DBasic = nullptr;

	BYTE beforeKey[256] = {};
	std::unique_ptr<DebugCamera> debugcamera = nullptr;

	bool useDebugcamera = false;

	std::mt19937 randomEngine{ std::random_device{}() };

	std::unique_ptr<SceneManager> sceneManager_ = nullptr;

};



