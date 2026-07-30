#include "WorldWeapon.h"
#include <numbers>

WorldWeapon::WorldWeapon(std::unique_ptr<Weapon> weapon, Vector3 pos, Rarity rarity, bool isForSale) :
	WorldItem(pos, rarity, isForSale),
	weapon_(std::move(weapon)) {

	weapon_->GetModel()->SetTranslate(pos);

	if (weapon_ && weapon_->GetData().name == "Spellbook") {
	} else {
		weapon_->GetModel()->SetRotate({ 0,float(std::numbers::pi) / 2.0f,0 });
	}

	if (isForSale_) {
		auto& ctx = GameContext::GetInstance();
		price_ = int(price_ * ctx.RandomFloat(valueRangeMin_, valueRangeMax_));
	}
}

std::unique_ptr<Weapon> WorldWeapon::GetWeapon() {
	return std::move(weapon_);
}

void WorldWeapon::Update() {
	auto model = weapon_->GetModel();
	auto shadow = weapon_->GetShadowModel();
	model->SetTranslate({ position_.x, 0.5f, position_.y });
	shadow->SetTranslate({ position_.x, 0.5f, position_.y });
	shadow->SetRotate(model->GetTransform().rotate);
}

void WorldWeapon::Draw() {
	WorldItem::Draw();
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawModel(weapon_->GetModel());
	render.DrawModel(weapon_->GetShadowModel());
}

void WorldWeapon::OnPickup(Player* player) {
	if (isForSale_) {
		Wallet& wallet = player->GetWallet();
		if (!wallet.Pay(price_)) {
			// 所持金不足
			return;
		}
	}

	// プレイヤーに持たせる
	player->SetWeapon(this->GetWeapon());

	// ワールド上からはなくなる
	isDead_ = true;
}
