#pragma once
#include "Item/Passive/Passive.h"

class Counter : public Passive{
public:
	Counter(Sprite* sprite);
	void OnHit(const Vector2& pos, BulletManager* bulletManager, Character* from) override;
};

