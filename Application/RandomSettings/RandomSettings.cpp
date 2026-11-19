#include "RandomSettings.h"
#include "Map/FloorSetting.h"
#include "GameCommon.h"

namespace {
	// 部屋タイプごとの敵の数
	int EnemyCount(RoomType type, int depth) {
		if (type == RoomType::Boss || type == RoomType::Elite) { return 1; }

		int n = 3;
		if (depth > 2) { n++; }
		if (depth > 4) { n++; }
		if (depth > 6) { n++; }
		return n;
	}
}

RandomSettings::RandomSettings() {
	enemySpawn_ = std::make_unique<EnemySpawnSetting>();

	Update();
}

void RandomSettings::Update() {
	table_.enemy = enemySpawn_->GetSpawnData();
}

std::string RandomSettings::SelectEnemy(int depth, RoomType type) {
	if (type == RoomType::Boss) { // ボスの階層は FloorConfig で決まるので、階層ではなく部屋タイプで見る
		enemySpawn_->Load("Resources/Data/Spawn/Enemy/Boss.json");
	} else if (type == RoomType::Elite) {
		enemySpawn_->Load("Resources/Data/Spawn/Enemy/Elite.json");
	} else if (depth <= kEarlyFloorNumber) {
		enemySpawn_->Load("Resources/Data/Spawn/Enemy/First.json");
	} else {
		enemySpawn_->Load("Resources/Data/Spawn/Enemy/Standard.json");
	}
	table_.enemy = enemySpawn_->GetSpawnData();

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
				return entry.name;
			}
		}
	}

	return "";
}

RoomPlan RandomSettings::PlanRoom(RoomType type, int depth, const FloorConfig& config) {
	RoomPlan plan;
	plan.planned = true;

	// 戦闘のない部屋(ショップ・ゴールなど)は空の予定
	if (type != RoomType::Combat && type != RoomType::Elite && type != RoomType::Boss) {
		return plan;
	}

	const int count = EnemyCount(type, depth);
	for (int i = 0; i < count; ++i) {
		std::string name = SelectEnemy(depth, type);
		if (!name.empty()) { plan.enemies.push_back(name); }
	}

	// 報酬(種類・レアリティ・額は FloorConfig の確率から。ボスも同じ)
	auto& ctx = GameContext::GetInstance();
	plan.reward = config.ChooseReward(type, ctx.RandomFloat(0.0f, 1.0f), ctx.RandomFloat(0.0f, 1.0f));
	return plan;
}