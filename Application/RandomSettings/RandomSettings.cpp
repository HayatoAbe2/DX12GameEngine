#include "RandomSettings.h"
#include "GameCommon.h"

RandomSettings::RandomSettings() {
	enemySpawn_ = std::make_unique<EnemySpawnSetting>();

	Update();
}

void RandomSettings::Update() {
	table_.enemy = enemySpawn_->GetSpawnData();
}

EnemyType RandomSettings::SelectEnemy() {
	// 合計重み
	float totalWeight = 0.0f;
	for (const auto& entry : table_.enemy) {
		totalWeight += entry.weight;
	}

	if (totalWeight > 0.0f) {
		auto& ctx = GameContext::GetInstance();
		float random = ctx.RandomFloat(0.0f, totalWeight);

		for (const auto& entry : table_.enemy) {
			random -= entry.weight;

			// 結果
			if (random <= 0.0f) {
				return entry.type;
			}
		}
	}

	return EnemyType::Fly;
}
