#include <Windows.h>
#include "2025_CG2_DirectX/engine/debug/D3DResourceLeakChecker.h"
#include "2025_CG2_DirectX/engine/Engine.h"
#include "2025_CG2_DirectX/application/Game.h"

//Windowsアプリでのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// DirectXのリソースリークチェック
	D3DResourceLeakChecker leakCheck;

	// エンジンを継承してゲームを作る
	Engine* game = new Game();

	game->Run();

	delete game;

	return 0;
}