#pragma once
#include "Objects/Item/Passive/Passive.h"

/// <summary>
/// 敵を倒した時回復
/// </summary>
class Heal final : public Passive {
public:
	Heal(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnEliminate(const Vector2& enemyPos, Character* self) override;

private:
	int kHealAmount = 5;
};

