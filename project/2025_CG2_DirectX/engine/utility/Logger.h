#pragma once
#include <string>
#include <fstream>

class Logger
{
public:

	void Initialize();

	void Update();

	void Log(const std::string& message);


private:

	//ログファイル
	std::ofstream logstream;

};

