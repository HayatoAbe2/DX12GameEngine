#include "Weapon.h"

Weapon::Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	data_ = data;
	charge_ = data_.stats.maxCharge;

	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel); 
	chargeStartTimer_ = std::make_unique<Timer>();
}

void Weapon::Update() {
	chargeStartTimer_->Update();
	if (chargeStartTimer_->IsFinished()) {
		float deltatime = GameContext::GetInstance().GetDeltatime();
		charge_ = (std::min)(data_.stats.maxCharge, charge_ + deltatime / data_.stats.chargeTime);
	}
}

void Weapon::SetChargeStartTimer() {
	chargeStartTimer_->Start(0.5f);
}
