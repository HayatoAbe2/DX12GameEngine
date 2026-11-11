#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Objects/Weapon/WeaponStatus.h"
#include "Editor/EditHistory.h"

/// <summary>
/// 武器エディタ(ImGui)。Resources/Data/Weapons のjsonを編集して保存する
/// </summary>
class WeaponEditor {
public:
	WeaponEditor();
	void Draw();

	// 保存・削除した後に呼ばれる(ゲームの武器データの再読み込み用)
	void SetOnApplied(std::function<void()> onApplied) { onApplied_ = std::move(onApplied); }

private:
	// 編集対象1つ分(1つのキー = 1つの武器)
	struct Entry {
		std::string key;      // 武器名(jsonのキー。敵用は末尾が _Enemy)
		std::string path;     // 保存先ファイル
		WeaponData data;      // 編集中のデータ
		EditHistory history;
		bool isNew = false;   // まだファイルに無い
	};

	// 読み込み・一覧
	void Scan();
	void SortEntries();
	Entry* Selected();

	// 画面
	void DrawList();
	void DrawEditor(Entry& entry);
	void DrawPreview(const WeaponData& weapon);
	void DrawPopups();

	// 操作
	void AddNew(const std::string& key);
	void Duplicate();
	void DeleteSelected();
	bool Save(Entry& entry);
	void Undo(Entry& entry);
	void Redo(Entry& entry);
	void ApplySnapshot(Entry& entry, const std::string& snapshot);

	// 新しい名前の確認(問題がなければ空文字)
	std::string CheckNewKey(const std::string& key, bool isEnemy) const;
	static std::string MakeKey(const std::string& name, bool isEnemy);
	static std::string Snapshot(const WeaponData& weapon);

	std::vector<Entry> entries_;
	int selected_ = -1;
	std::vector<std::string> models_;   // 選べるモデル名

	// ポップアップ用
	std::string newName_;
	bool newIsEnemy_ = false;
	bool openNewPopup_ = false;
	bool openDeletePopup_ = false;
	bool openReloadPopup_ = false;

	// 直前の操作結果
	std::string status_;

	std::function<void()> onApplied_;
};
