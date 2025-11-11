#pragma once
#include <string>
#include <fstream>

class Logger
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

	/// <summary>
	/// ログを出力
	/// </summary>
	/// <param name="message">メッセージ</param>
	void Log(const std::string& message);


private:

	/* --------- private変数 --------- */

	// ログファイル
	std::ofstream logstream_;

};

