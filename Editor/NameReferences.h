#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "Editor/EditorFile.h"

/// <summary>
/// 名前変更のとき、その名前を参照している他のデータファイルを探して書き換える
/// ・敵用の武器名 → Enemies.json の各敵の "weapons"
/// ・敵の名前     → Resources/Data/Spawn/Enemy/*.json の "spawnRates[].name"
/// </summary>
namespace NameReferences {

	constexpr const char* kEnemyFile = "Resources/Data/Enemies/Enemies.json";
	constexpr const char* kSpawnDir = "Resources/Data/Spawn/Enemy";

	struct Result {
		bool ok = true;                       // false なら1つも書き換えていない
		std::vector<std::string> locations;   // 参照していた場所(表示用)
		std::string error;
	};

	namespace detail {
		// 値が oldName の文字列を newName に置き換える(newName が空なら数えるだけ)
		inline int ReplaceInArray(nlohmann::ordered_json& array, const std::string& oldName, const std::string& newName, const char* field) {
			int count = 0;
			if (!array.is_array()) { return 0; }
			for (auto& item : array) {
				nlohmann::ordered_json* target = &item;
				if (field) {
					if (!item.is_object() || !item.contains(field)) { continue; }
					target = &item[field];
				}
				if (target->is_string() && target->get<std::string>() == oldName) {
					if (!newName.empty()) { *target = newName; }
					++count;
				}
			}
			return count;
		}
	}

	// 敵用の武器名(末尾の _Enemy を除いたもの)を使っている敵を探す。apply が true なら書き換えて保存する
	inline Result FindOrRenameWeaponUsers(const std::string& oldName, const std::string& newName, bool apply) {
		Result result;
		nlohmann::ordered_json root;
		if (!EditorFile::ReadJson(kEnemyFile, root)) {
			result.ok = false;
			result.error = std::string("読めないファイルがあります: ") + kEnemyFile;
			return result;
		}
		if (!root.is_object()) { return result; }

		bool changed = false;
		for (auto& [enemyName, enemy] : root.items()) {
			if (!enemy.is_object() || !enemy.contains("weapons")) { continue; }
			const int n = detail::ReplaceInArray(enemy["weapons"], oldName, apply ? newName : std::string(), nullptr);
			if (n > 0) {
				result.locations.push_back("敵「" + enemyName + "」の武器");
				changed = true;
			}
		}
		if (apply && changed && !EditorFile::WriteJson(kEnemyFile, root)) {
			result.ok = false;
			result.error = std::string("書き込めません: ") + kEnemyFile;
		}
		return result;
	}

	// 敵の名前を使っている出現率ファイルを探す。apply が true なら書き換えて保存する
	// (途中で失敗したときは、書き換え済みのファイルを元に戻す)
	inline Result FindOrRenameEnemyUsers(const std::string& oldName, const std::string& newName, bool apply) {
		Result result;
		struct Pending { std::string path; nlohmann::ordered_json before; nlohmann::ordered_json after; };
		std::vector<Pending> pending;

		for (const std::string& fileName : EditorFile::ListFiles(kSpawnDir, { ".json" })) {
			const std::string path = (std::filesystem::path(kSpawnDir) / fileName).generic_string();
			nlohmann::ordered_json root;
			if (!EditorFile::ReadJson(path, root)) {
				result.ok = false;
				result.error = "読めないファイルがあります: " + path;
				return result;
			}
			if (!root.is_object() || !root.contains("spawnRates")) { continue; }

			nlohmann::ordered_json after = root;
			const int n = detail::ReplaceInArray(after["spawnRates"], oldName, apply ? newName : std::string(), "name");
			if (n > 0) {
				result.locations.push_back("出現率「" + fileName + "」");
				pending.push_back({ path, std::move(root), std::move(after) });
			}
		}

		if (!apply) { return result; }

		for (size_t i = 0; i < pending.size(); ++i) {
			if (!EditorFile::WriteJson(pending[i].path, pending[i].after)) {
				for (size_t k = 0; k < i; ++k) { EditorFile::WriteJson(pending[k].path, pending[k].before); }
				result.ok = false;
				result.error = "書き込めません: " + pending[i].path;
				return result;
			}
		}
		return result;
	}
}
