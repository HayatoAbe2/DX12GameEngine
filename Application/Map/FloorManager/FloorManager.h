#pragma once
#include "FloorGenerator/FloorGenerator.h"
#include "Map/FloorConfig.h"
#include "Data/DataIssue.h"
#include <string>
#include <vector>

class RandomSettings;
class EnemyManager;

/// <summary>
/// 部屋と進度管理
/// </summary>
class FloorManager {
public:
	void Initialize();
	void Update(EnemyManager* enemyManager);

	void LoadNextRoom(Direction exitDir);
	Vector2 GetStartPos();
	void Reset();

	int GetCurrentDepth() { return currentDepth_; }
	// 今のフロアのゴールの階層(フロアを作ったときの設定で決まる。途中で設定を変えてもずれない)
	int GetMaxDepth() const { return maxDepth_; }
	RoomType GetCurrentType() const { return currentRoom_ ? currentRoom_->type : RoomType::Start; }
	std::vector<RoomConnector> GetConnector();
	RoomState GetCurrentRoomState() { return currentRoom_->state; }

	// 部屋の内容生成
	void PlanNextRooms(RandomSettings* random);
	const RoomPlan& GetCurrentPlan() const {
		static const RoomPlan kEmpty;
		return currentRoom_ ? currentRoom_->plan : kEmpty;
	}

	// フロア設定(FloorEditor が保存するファイル)を読み直す。壊れている・エラーがあるときは今の設定のまま
	bool ReloadConfig(std::vector<DataIssue>* issues = nullptr);
	const FloorConfig& GetConfig() const { return config_; }

	// 部屋の予定をファイルへ出す・ファイルの内容を反映する(デバッグ用)
	bool SaveFloorPlan(const std::string& path) const;
	bool LoadFloorPlan(const std::string& path, std::vector<DataIssue>* issues = nullptr);
	const Room* GetRoot() const { return rootRoom_; }

private:
	std::unique_ptr<FloorGenerator> generator_ = nullptr;
	Room* currentRoom_ = nullptr;
	Room* rootRoom_ = nullptr; // 最初の部屋(currentRoom_ は進んでいく)
	int currentDepth_ = 0;
	int maxDepth_ = 10;
	FloorConfig config_;
};

