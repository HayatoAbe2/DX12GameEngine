#include "EnemySpawnSetting.h"

EnemySpawnSetting::EnemySpawnSetting() {
	// 仮置き
	spawnData_.push_back({ EnemyType::Fly, 1.0f });
	spawnData_.push_back({ EnemyType::Golem, 1.0f });
	spawnData_.push_back({ EnemyType::Hedgehog, 1.0f });
}

void EnemySpawnSetting::Load() {
}
