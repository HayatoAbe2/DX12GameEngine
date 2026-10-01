#pragma once
#include <memory>
#include "EnemySpawn/EnemySpawnSetting.h"

struct SpawnTable {
	std::vector<EnemySpawn> enemy;
};

class RandomSettings {
public:
	RandomSettings();

	// 確率更新
	void Update();

	// 敵を抽選
	EnemyType SelectEnemy();

private:
	std::unique_ptr<EnemySpawnSetting> enemySpawn_;

	SpawnTable table_;
};

