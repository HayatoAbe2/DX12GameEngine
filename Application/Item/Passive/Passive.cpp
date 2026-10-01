#include "Passive.h"

Passive::Passive(std::unique_ptr<Sprite> sprite, PassiveContext ctx) {
	sprite_ = std::move(sprite);
	sprite_->SetPivot({ 0.5f,0.5f });
	sprite_->SetSize({ 88.0f,88.0f });

	passiveCtx_ = ctx;
}

void Passive::Draw() {
	GameContext::GetInstance().Render().DrawSprite(sprite_.get());
}
