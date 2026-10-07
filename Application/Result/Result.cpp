#include "Result.h"
#include "Map/FloorManager/FloorManager.h"

Result::Result(Fade* fade) {
	fade_ = fade;

	auto& ctx = GameContext::GetInstance();
	auto asset = ctx.Asset();
	resultBG_ = asset.LoadSprite("resources/Images/Result/result.png");
	resultBG_->SetColor({ 1, 1, 1, 0.7f });

	resultCursor_ = asset.LoadSprite("resources/Images/Result/cursor.png");
	resultCursor_->SetSize({ 48,56 });
	resultCursor_->SetColor({ 1, 1, 1, 0.7f });
	resultCursor_->SetPivot({ 0.5f,0.5f });
}

void Result::Update(FloorManager* floor) {
	auto& ctx = GameContext::GetInstance();
	auto input = ctx.Input();
	auto scene = ctx.Scene();

	switch (fade_->GetPhase()) {
	case FadePhase::None:
	{
		if (resultTime_ == 0) {
			resultTimer_.Start(2.0f);
		}

		// リザルト
		if (resultArrowMove_ < 1.0f) {
			resultArrowMove_ = max(resultArrowMove_ + ctx.GetDeltatime() * 1, 1.0f);
		}

		resultTimer_.Update();
		resultTime_ += ctx.GetDeltatime();

		float left = 227.0f;
		float width = 1053.0f - left;
		float endX = left + (float(floor->GetCurrentDepth()) / float(floor->kMaxDepth)) * width;

		float sinWave_ = sinf(10.0f * float(std::numbers::pi) * resultTime_ * 0.3f);
		resultCursor_->SetPosition({ endX * resultArrowMove_, 180 + sinWave_ * 10 });

		if (resultTimer_.IsFinished() &&
			(input.keyboard.IsRelease(DIK_SPACE) || input.gamepad.IsRelease(XINPUT_GAMEPAD_A))) {
			fade_->StartFadeOut();
			return;
		}
	}
	break;
	case FadePhase::Faded:
		scene.SceneChange("Game");
	}
}

void Result::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	resultBG_->SetSize(ctx.GetRenderWindowSize());
	render.DrawSprite(resultBG_.get());
	render.DrawSprite(resultCursor_.get());
}
