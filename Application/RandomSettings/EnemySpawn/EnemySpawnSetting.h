#pragma once
#include <vector>
#include <string>
#include "Character/Enemy/EnemyType.h"

struct EnemySpawn {
	EnemyType type;
	float weight = 0;
};

class EnemySpawnSetting {
public:
	EnemySpawnSetting();

	// ファイル読み込み
	void Load(std::string filePath);

	const std::vector<EnemySpawn>& GetSpawnData() { return spawnData_; }

private:
	// 出現データ
	std::vector<EnemySpawn> spawnData_;
};

