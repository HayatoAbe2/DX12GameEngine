#pragma once
#include <memory>
#include "EnemySpawn/EnemySpawnSetting.h"

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
	EnemyType SelectEnemy(int depth);

private:
	std::unique_ptr<EnemySpawnSetting> enemySpawn_;

	SpawnTable table_;
};

