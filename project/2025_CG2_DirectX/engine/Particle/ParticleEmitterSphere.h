#pragma once
#include <Vector3.h>
#include <wrl/client.h>
#include <DirectXBasic.h>

class ParticleEmitterSphere {
public:

	/* --------- namespace省略 --------- */

	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	void Initialize();

	void Update();

	void Draw();

	void DebugDraw();

private:
};

