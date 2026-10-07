#pragma once
#include "Item/Passive/Passive.h"

class ReloadBoost : public Passive{
public:
	ReloadBoost(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnUpdate(Weapon* weapon, Weapon* subWeapon) override;
};

