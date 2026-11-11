#pragma once
#include <string>

/// <summary>
/// データ検証の結果(エディタに表示する警告・エラー)
/// </summary>
struct DataIssue {
	// true: 保存できないエラー / false: 保存はできる警告
	bool isError = false;
	std::string message;
};
