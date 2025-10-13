#pragma once
#include <string>
#include <fstream>

class Logger
{
public:

	/* --------- public関数 --------- */

	void Initialize();

	void Update();

	void Log(const std::string& message);


private:

	/* --------- private変数 --------- */

	// ログファイル
	std::ofstream logstream;

};

