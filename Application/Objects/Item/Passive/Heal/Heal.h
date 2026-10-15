#pragma once
#include "Objects/Item/Passive/Passive.h"

class Heal : public Passive {
public:
	Heal(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnEliminate(const Vector2& enemyPos, Character* self) override;

private:
	int kHealAmount = 5;
};

