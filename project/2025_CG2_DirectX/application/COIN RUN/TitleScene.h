#pragma once

#include "../engine/Scene/BaseScene.h"

#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>

class Camera;
class Light;
class Sprite;
struct CameraForGPU;
struct LightBuffer;

class TitleScene : public BaseScene
{
public:
	TitleScene();
	~TitleScene() override;

	void Initialize(DirectXBasic* directXBasic, Object3DBasic* object3dBasic,
		SkinnedObject3DBasic* skinnedObject3dBasic, ModelManager* modelManager,
		Logger* logger, SRVManager* srvManager, TextureManager* textureManager,
		SpriteBasic* spriteBasic, XAudio2Basic* xaudio2Basic,
		std::mt19937* randomEngine) override;

	void Update() override;
	void SpriteDraw() override;
	void ModelDraw() override;
	void SkinnedModelDraw() override;
	void ImGuiDraw() override;
	void Finalize() override;

private:
	DirectXBasic* directXBasic_ = nullptr;
	std::unique_ptr<Camera> camera_;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	CameraForGPU* cameraForGPUData_ = nullptr;

	std::vector<std::unique_ptr<Light>> lights_;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightsBufferResource_;
	LightBuffer* lightsBufferData_ = nullptr;

	std::unique_ptr<Sprite> titleLogoSprite_;
};
