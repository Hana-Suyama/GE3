#pragma once
#include <chrono>

class FixFPS
{
public:

	void Initialize();

	void Update();

private:

	std::chrono::steady_clock::time_point reference_;

};

