#include "Weapon.h"

Weapon::Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	data_ = data;
	charge_ = data_.stats.maxCharge;

	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel); 
}
