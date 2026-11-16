#pragma once
#include <memory>
#include "EnemySpawn/EnemySpawnSetting.h"
#include "Map/Room/RoomData.h"

/// <summary>
/// 敵出現テーブル
/// </summary>
struct SpawnTable {
	std::vector<EnemySpawn> enemy;
};

/// <summary>
/// ランダム系の統括
/// </summary>
class RandomSettings {
public:
	RandomSettings();

	// 確率更新
	void Update();

	// 敵を抽選
	std::string SelectEnemy(int depth, RoomType type = RoomType::Combat);

	// 部屋内容
	RoomPlan PlanRoom(RoomType type, int depth);
	// 報酬の設定
	RewardPlan SelectReward(RoomType type);

private:
	std::unique_ptr<EnemySpawnSetting> enemySpawn_;

	SpawnTable table_;
};

