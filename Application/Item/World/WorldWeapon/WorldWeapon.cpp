#include "WorldWeapon.h"
#include <numbers>

WorldWeapon::WorldWeapon(std::unique_ptr<Weapon> weapon, Vector3 pos, Rarity rarity) :
	WorldItem(pos, rarity),
	weapon_(std::move(weapon)) {

	weapon_->GetModel()->SetTranslate(pos);

	if (weapon_ && weapon_->GetData().name == "Spellbook") {
	} else {
		weapon_->GetModel()->SetRotate({ 0,float(std::numbers::pi) / 2.0f,0 });
	}
}

std::unique_ptr<Weapon> WorldWeapon::GetWeapon() {
	return std::move(weapon_);
}

void WorldWeapon::Update() {
}

void WorldWeapon::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawModel(weapon_->GetModel());
	render.DrawModel(weapon_->GetShadowModel());
}

void WorldWeapon::OnPickup(Player* player) {
	// プレイヤーに持たせる
	player->SetWeapon(this->GetWeapon());

	// ワールド上からはなくなる
	isDead_ = true;
}
