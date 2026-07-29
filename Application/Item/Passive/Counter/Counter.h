#pragma once
#include "Item/Passive/Passive.h"

class Counter : public Passive{
public:
	Counter(std::unique_ptr<Sprite> sprite);
	void OnHit(const Vector2& pos, BulletManager* bulletManager, Character* from) override;
};

