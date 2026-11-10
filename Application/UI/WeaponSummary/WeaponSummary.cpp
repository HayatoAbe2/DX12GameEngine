#include "WeaponSummary.h"

WeaponSummaryUI::WeaponSummaryUI() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	background_ = asset.LoadSprite("Resources/Images/UI/WeaponSummary/Background.png");
	background_->SetPivot({ 0.5f,0.5f });

	video_ = asset.LoadVideo("Resources/Debug/videoTest.mp4", true);
	if (video_->IsOpen()) {
		videoSprite_ = asset.LoadSprite(video_->GetTexture());
		videoSprite_->SetSize(ctx.GetRenderWindowSize());
		video_->Play();
	}
}

void WeaponSummaryUI::Update(const WeaponData& data) {
	auto& ctx = GameContext::GetInstance();

	if (video_ && video_->IsOpen()) { video_->Update(ctx.GetDeltatime()); }
	isActive_ = true;
}
void WeaponSummaryUI::Update() {
	isActive_ = false;
}

void WeaponSummaryUI::Draw() {
	if (!isActive_) return;

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	videoSprite_->ImGuiEdit();
	background_->ImGuiEdit();

	render.DrawSprite(background_.get());
	if (videoSprite_) { render.DrawSprite(videoSprite_.get()); }
}
