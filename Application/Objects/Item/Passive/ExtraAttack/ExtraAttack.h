#pragma once
#include "Objects/Item/Passive/Passive.h"

/// <summary>
/// 撃破時に追加の弾発射
/// </summary>
class ExtraAttack final : public Passive {
public:
	ExtraAttack(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);
	void OnEliminate(const Vector2& enemyPos, Character* self) override;
};

