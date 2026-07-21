#include "ReloadBoost.h"

ReloadBoost::ReloadBoost(Sprite* sprite) : Passive(sprite) {
}

void ReloadBoost::OnUpdate(Weapon* weapon, Weapon* subWeapon) {
	if (subWeapon) {
		subWeapon->GetModifier().multiplier.shootCoolTime *= 0.7f;
	}
}
