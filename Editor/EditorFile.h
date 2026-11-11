#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <vector>
#include <Externals/nlohmann/json.hpp>
#include "Data/JsonFormat.h"

/// <summary>
/// エディタ用のファイル入出力
/// </summary>
namespace EditorFile {

	// フォルダ内の指定した拡張子のファイル名(ソート済み)
	inline std::vector<std::string> ListFiles(const std::string& dir, std::initializer_list<const char*> extensions) {
		std::vector<std::string> names;
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) { continue; }
			const std::string ext = entry.path().extension().string();
			for (const char* e : extensions) {
				if (ext == e) { names.push_back(entry.path().filename().string()); break; }
			}
		}
		std::sort(names.begin(), names.end());
		return names;
	}

	// JSONを読む。ファイルが無いときは空のオブジェクトで成功、壊れているときは失敗(false)
	inline bool ReadJson(const std::string& path, nlohmann::ordered_json& out) {
		out = nlohmann::ordered_json::object();
		std::ifstream file(path);
		if (!file.is_open()) { return true; }
		try {
			file >> out;
		} catch (...) {
			return false;
		}
		return true;
	}

	// JSONを保存する(フォルダが無ければ作る)
	inline bool WriteJson(const std::string& path, const nlohmann::ordered_json& j) {
		std::error_code ec;
		const std::filesystem::path p(path);
		if (p.has_parent_path()) { std::filesystem::create_directories(p.parent_path(), ec); }
		std::ofstream file(path);
		if (!file.is_open()) { return false; }
		file << DumpJson(j) << "\n";
		return file.good();
	}
}
