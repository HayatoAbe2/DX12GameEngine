#pragma once
#include "Item/Passive/Passive.h"

class Lightning : public Passive{
public:
	Lightning(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnDealDamage(const Vector2& pos) override;
};

