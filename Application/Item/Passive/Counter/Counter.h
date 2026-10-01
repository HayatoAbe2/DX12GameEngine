#pragma once
#include "Item/Passive/Passive.h"

class Counter : public Passive{
public:
	Counter(std::unique_ptr<Sprite> sprite, PassiveContext ctx);
	void OnHit(const Vector2& pos, Character* from) override;
};

