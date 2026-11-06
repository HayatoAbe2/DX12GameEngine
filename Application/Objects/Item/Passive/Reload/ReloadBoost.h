#pragma once
#include "Objects/Item/Passive/Passive.h"

/// <summary>
/// 構えていない武器のリロード加速
/// </summary>
class ReloadBoost : public Passive{
public:
	ReloadBoost(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnUpdate(Weapon* weapon, Weapon* subWeapon) override;
};

