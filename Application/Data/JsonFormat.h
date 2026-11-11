#pragma once
#include <regex>
#include <string>
#include <Externals/nlohmann/json.hpp>

/// <summary>
/// 保存用のJSON文字列を作る。インデントは4スペース、
/// 数値だけの配列(色など)は既存のファイルと同じく1行にまとめる。
/// </summary>
inline std::string DumpJson(const nlohmann::ordered_json& j) {
	const std::string text = j.dump(4);

	static const std::regex numberArray(R"(\[\s*(-?\d[^\[\]"]*?)\s*\])");
	static const std::regex separator(R"(\s*,\s*)");

	std::string out;
	out.reserve(text.size());
	size_t last = 0;
	for (auto it = std::sregex_iterator(text.begin(), text.end(), numberArray); it != std::sregex_iterator(); ++it) {
		const std::smatch& m = *it;
		out.append(text, last, size_t(m.position()) - last);
		out += "[" + std::regex_replace(m[1].str(), separator, ", ") + "]";
		last = size_t(m.position()) + size_t(m.length());
	}
	out.append(text, last, std::string::npos);
	return out;
}
