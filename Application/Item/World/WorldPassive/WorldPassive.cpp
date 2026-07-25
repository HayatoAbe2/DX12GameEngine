#include "WorldPassive.h"

WorldPassive::WorldPassive(std::unique_ptr<Passive> passive, Vector3 pos, Rarity rarity) :
	WorldItem(pos, rarity),
	passive_(std::move(passive)) {
}

std::unique_ptr<Passive> WorldPassive::GetPassiveItem() {
	return std::move(passive_);
}

void WorldPassive::Update() {
}

void WorldPassive::Draw() {
}

void WorldPassive::OnPickup(Player* player) {


	// ワールド上からはなくなる
	isDead_ = true;
}
