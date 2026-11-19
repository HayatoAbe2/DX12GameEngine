#include "Map/FloorConfig.h"
#include <algorithm>

namespace {

// 重みから1つ選ぶ。roll は 0以上1未満。重みが0以下の項目は選ばれない。全部0なら -1
int PickIndex(const float* weights, int count, float roll) {
	float total = 0.0f;
	for (int i = 0; i < count; ++i) {
		if (weights[i] > 0.0f) { total += weights[i]; }
	}
	if (total <= 0.0f) { return -1; }

	const float target = std::clamp(roll, 0.0f, 1.0f) * total;
	float accumulated = 0.0f;
	int last = -1;
	for (int i = 0; i < count; ++i) {
		if (weights[i] <= 0.0f) { continue; }
		accumulated += weights[i];
		last = i;
		if (target < accumulated) { return i; }
	}
	return last; // 誤差で届かなかったとき
}

} // namespace

std::optional<RoomType> FloorConfig::FixedTypeAt(int depth) const {
	for (const auto& fixed : fixedRooms) {
		if (fixed.depth == depth) { return fixed.type; }
	}
	return std::nullopt;
}

std::vector<float> FloorConfig::RoomTypeProbabilities(int depth, RoomType parent) const {
	std::vector<float> weights(roomRates.size(), 0.0f);
	float total = 0.0f;
	for (size_t i = 0; i < roomRates.size(); ++i) {
		const RoomTypeRate& rate = roomRates[i];
		if (depth < rate.minDepth) { continue; }

		float w = (std::max)(rate.weight, 0.0f);
		if (rate.type == parent) { w *= (std::max)(rate.repeatMultiplier, 0.0f); } // 連続の補正
		weights[i] = w;
		total += w;
	}

	if (total > 0.0f) {
		for (float& w : weights) { w /= total; }
	}
	return weights;
}

RoomType FloorConfig::RoomTypeAt(int depth, RoomType parent, float roll) const {
	if (depth >= goalDepth) { return RoomType::Goal; }

	if (const auto fixed = FixedTypeAt(depth)) { return *fixed; }

	const std::vector<float> p = RoomTypeProbabilities(depth, parent);
	const int index = PickIndex(p.data(), int(p.size()), roll);
	return index >= 0 ? roomRates[index].type : RoomType::Combat; // 抽選できる物がなければ戦闘
}

const RewardRate& FloorConfig::RewardOf(RoomType type) const {
	switch (type) {
	case RoomType::Elite: return eliteReward;
	case RoomType::Boss:  return bossReward;
	default:              return combatReward;
	}
}

RewardPlan FloorConfig::ChooseReward(RoomType type, float rollKind, float rollRarity) const {
	const RewardRate& rate = RewardOf(type);
	RewardPlan plan;

	const float kinds[3] = { rate.money, rate.weapon, rate.passive };
	const int kind = PickIndex(kinds, 3, rollKind);
	switch (kind) {
	case 1:
		plan.type = RewardType::Weapon;
		break;
	case 2:
		plan.type = RewardType::Passive;
		break;
	default: // お金(全て0のときもここ)
		plan.type = RewardType::Money;
		break;
	}

	if (plan.type == RewardType::Money) {
		plan.amount = rate.moneyAmount;
	} else if (plan.type == RewardType::Weapon) {
		const float rarities[4] = { rate.common, rate.rare, rate.epic, rate.legendary };
		static constexpr Rarity kRarities[4] = { Rarity::Common, Rarity::Rare, Rarity::Epic, Rarity::Legendary };
		const int index = PickIndex(rarities, 4, rollRarity);
		plan.rarity = index >= 0 ? kRarities[index] : Rarity::Common;
	}
	return plan;
}

const char* RoomTypeKey(RoomType type) {
	switch (type) {
	case RoomType::Start:  return "Start";
	case RoomType::Combat: return "Combat";
	case RoomType::Boss:   return "Boss";
	case RoomType::Elite:  return "Elite";
	case RoomType::Shop:   return "Shop";
	case RoomType::Event:  return "Event";
	case RoomType::Goal:   return "Goal";
	default:               return "Combat";
	}
}

const char* RoomTypeLabel(RoomType type) {
	switch (type) {
	case RoomType::Start:  return "スタート";
	case RoomType::Combat: return "戦闘";
	case RoomType::Boss:   return "ボス";
	case RoomType::Elite:  return "強敵";
	case RoomType::Shop:   return "ショップ";
	case RoomType::Event:  return "イベント";
	case RoomType::Goal:   return "ゴール";
	default:               return "?";
	}
}

const char* RoomFilePrefix(RoomType type) {
	switch (type) {
	case RoomType::Start:  return "startRoom";
	case RoomType::Combat: return "combatRoom";
	case RoomType::Boss:   return "bossRoom";
	case RoomType::Elite:  return "bossRoom";
	case RoomType::Shop:   return "shopRoom";
	case RoomType::Event:  return "eventRoom";
	case RoomType::Goal:   return "goalRoom";
	default:               return "combatRoom";
	}
}

bool ParseRoomType(const std::string& text, RoomType& out) {
	for (RoomType t : { RoomType::Start, RoomType::Combat, RoomType::Boss, RoomType::Elite,
						RoomType::Shop, RoomType::Event, RoomType::Goal }) {
		if (text == RoomTypeKey(t)) {
			out = t;
			return true;
		}
	}
	return false;
}
