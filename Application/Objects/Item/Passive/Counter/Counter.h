#pragma once
#include "Objects/Item/Passive/Passive.h"

/// <summary>
/// 被ダメージ時に反撃
/// </summary>
class Counter : public Passive{
public:
	Counter(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnHit(const Vector2& pos, Character* from) override;
};

