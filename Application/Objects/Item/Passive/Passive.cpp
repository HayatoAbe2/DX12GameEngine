#include "Passive.h"

Passive::Passive(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) {
	sprite_ = std::move(sprite);
	sprite_->SetPivot({ 0.5f,0.5f });
	sprite_->SetSize({ 88.0f,88.0f });

	// 説明
	if (explain) {
		explain_ = std::move(explain);
		explain_->SetPivot({ 0.5f,0.5f });
	}

	passiveCtx_ = ctx;
}

void Passive::Draw() {
	auto render = GameContext::GetInstance().Render();
	render.DrawSprite(sprite_.get());
	if (explain_) {
		render.DrawSprite(explain_.get());
	}
}
