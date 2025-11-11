#pragma once
#include <chrono>

class FixFPS
{
public:

	/* --------- public関数 --------- */

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private:

	/* --------- private変数 --------- */

	// 現在の時間
	std::chrono::steady_clock::time_point reference_;

};

