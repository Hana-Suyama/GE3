#pragma once
#include <chrono>

class FixFPS
{
public:

	/* --------- public関数 --------- */

	void Initialize();

	void Update();

private:

	/* --------- private変数 --------- */

	std::chrono::steady_clock::time_point reference_;

};

