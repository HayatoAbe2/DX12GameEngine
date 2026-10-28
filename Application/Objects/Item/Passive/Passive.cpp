#include "Passive.h"

Passive::Passive(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) {
	sprite_ = std::move(sprite);
	sprite_->SetPivot({ 0.5f,0.5f });
	sprite_->SetSize(kSpriteSize);

	// 説明
	if (explain) {
		description_ = std::move(explain);
		description_->SetPivot({ 0.5f,0.5f });
		description_->SetSize(description_->GetSize() * kDescriptionScale);

		background_ = GameContext::GetInstance().Asset().LoadSprite("Resources/Debug/white1x1.png");
		background_->SetPivot({ 0.5f,0.5f });
		background_->SetColor({ 0,0,0,0.5f });
		background_->SetSize(description_->GetSize() * kBackgroundScale);
	}

	passiveCtx_ = ctx;
}

void Passive::Draw() {
	auto render = GameContext::GetInstance().Render();
	render.DrawSprite(sprite_.get());
}

void Passive::DrawDescription() {
	auto render = GameContext::GetInstance().Render();
	if (description_) {
		render.DrawSprite(background_.get());
		render.DrawSprite(description_.get());
	}
}
