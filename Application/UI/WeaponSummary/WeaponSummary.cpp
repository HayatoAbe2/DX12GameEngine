#include "WeaponSummary.h"

WeaponSummaryUI::WeaponSummaryUI() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	summaryWindow_ = asset.LoadSprite("Resources/Images/UI/WeaponSummary/SummaryWindow.png");
	summaryWindow_->SetPivot({ 0.5f,0.5f });
	summaryItems_ = asset.LoadSprite("Resources/Images/UI/WeaponSummary/SummaryItems.png");
	summaryItems_->SetPivot({ 0.5f,0.5f });

	atkNums_ = std::make_unique<DrawNumbers>(kAtkDigitCount);
	maxBulletNums_ = std::make_unique<DrawNumbers>(kBulletDigitCount);
	reloadSpeedNumsL_ = std::make_unique<DrawNumbers>(kReloadDigitCountL);
	reloadSpeedNumsR_ = std::make_unique<DrawNumbers>(kReloadDigitCountR);
}

void WeaponSummaryUI::Update(const WeaponData& data) {
	auto& ctx = GameContext::GetInstance();
}
void WeaponSummaryUI::Update() {
	isActive_ = false;
}

void WeaponSummaryUI::Draw() {
	if (!isActive_) return;

	summaryWindow_->ImGuiEdit();
	summaryItems_->ImGuiEdit();

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
}
