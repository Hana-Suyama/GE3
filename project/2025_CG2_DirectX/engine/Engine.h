#pragma once

#include "PostEffectController.h"
#include "MyMath.h"

#include <memory>
#include <random>

class TextureManager;
class ModelManager;
class XAudio2Basic;
class Logger;
class WindowsApi;
class DirectXBasic;
class SRVManager;
class ImGuiManager;
class SpriteBasic;
class Camera;
class Object3DBasic;
class SkinnedObject3DBasic;
class PostEffectRenderer;
class DebugCamera;
class SceneManager;

using namespace MyMath;

#pragma comment(lib, "dxguid.lib")

class Engine
{
public:

	Engine();
	virtual ~Engine();

	virtual void Initialize();

	virtual void Update();

	virtual void Draw() = 0;

	void RenderTexturePreDraw();
	void BackBufferPreDraw();
	void SceneSpriteDraw();
	void SceneModelDraw();
	void DrawPostEffect();
	void PostDraw();
	void PostEffectDebugDraw();

	virtual void Finalize();

	void Run();

	void SetPostEffectType(PostEffectType type){ postEffectController_.SetType(type); }

protected:

	std::unique_ptr<Logger> logger_;
	std::unique_ptr<WindowsApi> winApi_;
	std::unique_ptr<DirectXBasic> directXBasic_;
	std::unique_ptr<SRVManager> srvManager_;
	std::unique_ptr<ImGuiManager> imguiManager_;

	std::unique_ptr<TextureManager> textureManager_;
	std::unique_ptr<ModelManager> modelManager_;
	std::unique_ptr<XAudio2Basic> xaudio2Basic_;

	std::unique_ptr<SpriteBasic> spriteBasic_;
	std::unique_ptr<Camera> defaultCamera_;
	std::unique_ptr<Object3DBasic> object3DBasic_;
	std::unique_ptr<SkinnedObject3DBasic> skinnedObject3DBasic_;

	PostEffectController postEffectController_;
	std::unique_ptr<PostEffectRenderer> postEffectRenderer_;

	std::mt19937 randomEngine_{ std::random_device{}() };

	// 依存先より先に破棄する必要があるので最後に宣言
	std::unique_ptr<SceneManager> sceneManager_;
};



