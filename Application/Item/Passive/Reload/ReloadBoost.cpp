#include "ReloadBoost.h"

ReloadBoost::ReloadBoost(std::unique_ptr<Sprite> sprite) : Passive(std::move(sprite)) {
}

void ReloadBoost::OnUpdate(Weapon* weapon, Weapon* subWeapon) {
	if (subWeapon) {
		subWeapon->GetModifier()[int(ModifierStats::coolTime)].multiply *= 0.7f;
	}
}
