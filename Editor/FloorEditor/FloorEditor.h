#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Map/FloorConfig.h"
#include "Data/DataIssue.h"

/// <summary>
/// フロアエディタ(ImGui)。Resources/Data/Floor/FloorConfig.json を編集して保存する
/// ・固定設定: ゴールまでの移動回数、階層ごとに種類を固定する部屋(ボス・確定ショップなど)
/// ・確率設定: 部屋タイプの出現確率と連続の補正、報酬の確率(部屋タイプごと)
/// </summary>
class FloorEditor {
public:
	explicit FloorEditor(const std::string& filePath = kFloorConfigPath);

	void Draw();

	// 保存した後に呼ばれる(ゲームの設定の再読み込み用)
	void SetOnApplied(std::function<void()> onApplied) { onApplied_ = std::move(onApplied); }

	// 「フロアを作り直す」が押されたときに呼ばれる。設定すると、ボタンが出る
	void SetOnRegenerate(std::function<void()> onRegenerate) { onRegenerate_ = std::move(onRegenerate); }

private:
	void Load();
	bool Save();
	std::string Snapshot() const;
	std::vector<DataIssue> CollectIssues() const;

	void DrawFixedTab();
	void DrawRateTab();
	void DrawReward(const char* title, RoomType type);
	void DrawPopups();
	void AddFixedRoom();

	FloorConfig config_;
	std::string path_;
	std::string savedText_;   // 保存済みの内容(未保存の判定用)
	std::string status_;
	bool hasError_ = false;

	// 確率プレビュー
	int previewDepth_ = 1;
	RoomType previewParent_ = RoomType::Combat;

	bool openReloadPopup_ = false;
	bool openRegeneratePopup_ = false;

	std::function<void()> onApplied_;
	std::function<void()> onRegenerate_;
};
