#include "WorldPassive.h"

WorldPassive::WorldPassive(std::unique_ptr<Passive> passive, Vector3 pos, Rarity rarity, bool isForSale) :
	WorldItem(pos, rarity, isForSale),
	passive_(std::move(passive)) {

	if (isForSale_) {
		auto& ctx = GameContext::GetInstance();
		price_ = int(price_ * ctx.RandomFloat(valueRangeMin_, valueRangeMax_));
	}
}

std::unique_ptr<Passive> WorldPassive::GetPassiveItem() {
	return std::move(passive_);
}

void WorldPassive::Update() {
}

void WorldPassive::Draw() {
	WorldItem::Draw();
}

void WorldPassive::OnPickup(Player* player) {


	// ワールド上からはなくなる
	isDead_ = true;
}
