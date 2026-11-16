#include "RandomSettings.h"
#include "Map/FloorSetting.h"
#include "GameCommon.h"
#include <initializer_list>
#include <utility>

namespace {
	// 重み付き抽選
	template <class T>
	T PickWeighted(std::initializer_list<std::pair<T, float>> table) {
		float total = 0.0f;
		for (const auto& e : table) { total += e.second; }

		float r = GameContext::GetInstance().RandomFloat(0.0f, total);
		for (const auto& e : table) {
			r -= e.second;
			if (r <= 0.0f) { return e.first; }
		}
		return (table.end() - 1)->first;
	}

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
	if (type == RoomType::Boss || depth == kBossFloorNumber) {
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

RoomPlan RandomSettings::PlanRoom(RoomType type, int depth) {
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

	if (type == RoomType::Boss) {
		plan.reward.type = RewardType::Passive; // ボス報酬は固定
	} else {
		plan.reward = SelectReward(type);
	}
	return plan;
}

// 確率エディタができるまではここの数値を直接調整する
RewardPlan RandomSettings::SelectReward(RoomType type) {
	RewardPlan reward;
	if (type == RoomType::Elite) {
		reward.type = PickWeighted<RewardType>({
			{ RewardType::Money, 20.0f }, { RewardType::Weapon, 40.0f }, { RewardType::Passive, 40.0f } });
		reward.rarity = PickWeighted<Rarity>({
			{ Rarity::Common, 10.0f }, { Rarity::Rare, 50.0f }, { Rarity::Epic, 30.0f }, { Rarity::Legendary, 10.0f } });
		reward.amount = 70;
	} else {
		reward.type = PickWeighted<RewardType>({
			{ RewardType::Money, 40.0f }, { RewardType::Weapon, 40.0f }, { RewardType::Passive, 20.0f } });
		reward.rarity = PickWeighted<Rarity>({
			{ Rarity::Common, 70.0f }, { Rarity::Rare, 25.0f }, { Rarity::Epic, 5.0f } });
		reward.amount = 30;
	}
	return reward;
}