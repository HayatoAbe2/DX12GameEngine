#include "Fade.h"

Fade::Fade() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	sprite_ = asset.LoadSprite("resources/Debug/white1x1.png");

	// フェードイン開始
	timer_.Start(kMaxFadeinTimer_);
	fadePhase_ = FadePhase::FadeIn;
}

void Fade::Update() {
	timer_.Update();

	if (!timer_.IsActive()) {
		if(fadePhase_ == FadePhase::FadeIn) fadePhase_ = FadePhase::None;
		if(fadePhase_ == FadePhase::FadeOut) fadePhase_ = FadePhase::Faded;
	}
}

void Fade::Draw() {
	if (maxTime_ > 0) {
		auto& ctx = GameContext::GetInstance();
		auto& render = ctx.Render();

		float alpha = timer_.GetRemaining() / maxTime_;
		if (fadePhase_ == FadePhase::FadeOut) {
			alpha = 1.0f - alpha;
		}
		sprite_->SetColor({ 0,0,0,alpha });
		sprite_->SetSize(ctx.GetRenderWindowSize() + Vector2{ 20,80 });
		render.DrawSprite(sprite_.get());
	}
}

void Fade::StartFadeIn(float seconds) {
	maxTime_ = seconds;
	timer_.Start(seconds);
	fadePhase_ = FadePhase::FadeIn;
}

void Fade::StartFadeOut(float seconds) {
	maxTime_ = seconds;
	timer_.Start(seconds);
	fadePhase_ = FadePhase::FadeOut;
}
