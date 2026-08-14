#pragma once

#include "../engine/Scene/BaseScene.h"

class TitleScene : public BaseScene
{
public:
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
	std::unique_ptr<Sprite> titleLogoSprite_;
};
