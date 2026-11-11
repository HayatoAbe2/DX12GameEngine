#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Objects/Character/Enemy/EnemyData.h"
#include "Editor/EditHistory.h"

struct WeaponData;

/// <summary>
/// 敵エディタ(ImGui)。Resources/Data/Enemies/Enemies.json を編集して保存する
/// </summary>
class EnemyEditor {
public:
	// 敵用武器の名前からデータを引く(無ければ nullptr)
	using WeaponLookup = std::function<const WeaponData*(const std::string&)>;

	EnemyEditor();

	// enemyWeaponNames: 選べる敵用の武器名(WeaponManager::GetEnemyAttackNames)
	void Draw(const std::vector<std::string>& enemyWeaponNames, const WeaponLookup& findWeapon);

	// 保存・削除した後に呼ばれる(ゲームの敵データの再読み込み用)
	void SetOnApplied(std::function<void()> onApplied) { onApplied_ = std::move(onApplied); }

	// 「出す」ボタンが押されたときに呼ばれる(引数は敵の名前)。設定するとボタンが出る
	void SetOnSpawn(std::function<void(const std::string&)> onSpawn) { onSpawn_ = std::move(onSpawn); }

private:
	// 編集対象1つ分(1つのキー = 1種類の敵)
	struct Entry {
		std::string key;      // 敵の名前(jsonのキー)
		EnemyData data;
		EditHistory history;
		bool isNew = false;   // まだファイルに無い
	};

	void Scan();
	Entry* Selected();

	void DrawList();
	void DrawEditor(Entry& entry, const std::vector<std::string>& weaponNames, const WeaponLookup& findWeapon);
	void DrawPopups();

	void AddNew(const std::string& key);
	void Duplicate();
	void DeleteSelected();
	bool Save(Entry& entry);
	void Undo(Entry& entry);
	void Redo(Entry& entry);
	void ApplySnapshot(Entry& entry, const std::string& snapshot);

	std::string CheckNewKey(const std::string& key) const;
	static std::string Snapshot(const EnemyData& data);

	std::vector<Entry> entries_;   // ファイルの並びのまま
	int selected_ = -1;
	std::vector<std::string> models_;

	std::string newName_;
	bool openNewPopup_ = false;
	bool openDeletePopup_ = false;
	bool openReloadPopup_ = false;

	std::string status_;
	std::function<void()> onApplied_;
	std::function<void(const std::string&)> onSpawn_;
};
