#pragma once
#include <optional>
#include <string>
#include <vector>

/// <summary>
/// エディタの編集履歴(Undo/Redo)と、保存済みとの差分判定。
/// 内容は文字列(JSONのスナップショット)で受け渡す。
/// </summary>
struct EditHistory {
	std::string saved;                // 保存済みの内容
	std::string last;                 // 直前の内容(現在の状態)
	std::vector<std::string> undo;
	std::vector<std::string> redo;
	bool dirty = false;               // 変更した

	// 読み込み直後・保存直後などに履歴を作り直す
	void Reset(const std::string& snapshot) {
		saved = last = snapshot;
		undo.clear();
		redo.clear();
		dirty = false;
	}

	// 操作が終わったタイミングで呼ぶ。変更があれば履歴に積む
	void Commit(const std::string& now) {
		if (now == last) { return; }
		undo.push_back(last);
		if (undo.size() > kMax) { undo.erase(undo.begin()); }
		redo.clear();
		last = now;
		dirty = (last != saved);
	}

	void MarkSaved() {
		saved = last;
		dirty = false;
	}

	// 戻した後の内容を返す(戻せなければ空)
	std::optional<std::string> Undo() {
		if (undo.empty()) { return std::nullopt; }
		redo.push_back(last);
		last = undo.back();
		undo.pop_back();
		dirty = (last != saved);
		return last;
	}

	std::optional<std::string> Redo() {
		if (redo.empty()) { return std::nullopt; }
		undo.push_back(last);
		last = redo.back();
		redo.pop_back();
		dirty = (last != saved);
		return last;
	}

private:
	static constexpr size_t kMax = 100;
};
