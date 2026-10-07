#include "ReloadBoost.h"

ReloadBoost::ReloadBoost(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) : Passive(std::move(sprite), std::move(explain), passiveCtx_) {
}

void ReloadBoost::OnUpdate(Weapon* weapon, Weapon* subWeapon) {
	if (subWeapon) {
		subWeapon->GetModifier()[int(ModifierStats::coolTime)].multiply *= 0.7f;
	}
}
