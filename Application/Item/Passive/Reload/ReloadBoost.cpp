#include "ReloadBoost.h"

ReloadBoost::ReloadBoost(std::unique_ptr<Sprite> sprite, PassiveContext ctx) : Passive(std::move(sprite), passiveCtx_) {
}

void ReloadBoost::OnUpdate(Weapon* weapon, Weapon* subWeapon) {
	if (subWeapon) {
		subWeapon->GetModifier()[int(ModifierStats::coolTime)].multiply *= 0.7f;
	}
}
