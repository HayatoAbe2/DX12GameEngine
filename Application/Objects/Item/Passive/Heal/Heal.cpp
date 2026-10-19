#include "Heal.h"

Heal::Heal(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) : Passive(std::move(sprite), std::move(explain), ctx) {
}

void Heal::OnEliminate(const Vector2& enemyPos, Character* self) {
	self->Heal(kHealAmount);
}