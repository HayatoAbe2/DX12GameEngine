#include "Passive.h"

Passive::Passive(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) {
	sprite_ = std::move(sprite);
	sprite_->SetPivot({ 0.5f,0.5f });
	sprite_->SetSize(kSpriteSize);

	// 説明
	if (explain) {
		explain_ = std::move(explain);
		explain_->SetPivot({ 0.5f,0.5f });
		explain_->SetSize(explain_->GetSize() * kExplainScale);

		background_ = GameContext::GetInstance().Asset().LoadSprite("Resources/Debug/white1x1.png");
		background_->SetPivot({ 0.5f,0.5f });
		background_->SetColor({ 0,0,0,0.5f });
		background_->SetSize(explain_->GetSize() * kBackgroundScale);
	}

	passiveCtx_ = ctx;
}

void Passive::Draw() {
	auto render = GameContext::GetInstance().Render();
	render.DrawSprite(sprite_.get());
}

void Passive::DrawExplain() {
	auto render = GameContext::GetInstance().Render();
	if (explain_) {
		render.DrawSprite(background_.get());
		render.DrawSprite(explain_.get());
	}
}
