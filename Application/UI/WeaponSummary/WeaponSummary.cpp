#include "WeaponSummary.h"

WeaponSummaryUI::WeaponSummaryUI() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	background_ = asset.LoadSprite("Resources/Images/UI/WeaponSummary/Background.png");
	background_->SetPivot({ 0.5f,0.5f });
}

void WeaponSummaryUI::Update(const WeaponData& data) {
	auto& ctx = GameContext::GetInstance();
}
void WeaponSummaryUI::Update() {
	isActive_ = false;
}

void WeaponSummaryUI::Draw() {
	if (!isActive_) return;

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
}
