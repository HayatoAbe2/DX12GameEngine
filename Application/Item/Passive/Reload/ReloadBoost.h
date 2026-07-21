#pragma once
#include "Item/Passive/Passive.h"

class ReloadBoost : public Passive{
public:
	ReloadBoost(Sprite* sprite);
	void OnUpdate(Weapon* weapon, Weapon* subWeapon) override;
};

