#pragma once
#include <string>

namespace StringUtility
{
	/// <summary>
	/// string→wstringに変換
	/// </summary>
	/// <param name="str">文字列</param>
	std::wstring ConvertString(const std::string& str);

	/// <summary>
	/// wstring→stringに変換
	/// </summary>
	/// <param name="str">文字列</param>
	std::string ConvertString(const std::wstring& str);
};

