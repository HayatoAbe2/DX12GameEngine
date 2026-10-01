#pragma once
#include "Item/Passive/Passive.h"

class Lightning : public Passive{
public:
	Lightning(std::unique_ptr<Sprite> sprite, PassiveContext ctx);
	void OnDealDamage(const Vector2& pos) override;
};

