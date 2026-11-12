#include "MoneyUI.h"
#include "Wallet/Wallet.h"

MoneyUI::MoneyUI(Wallet& wallet) : wallet_(wallet) {
	wallet_ = wallet;
	wallet_.Initialize(this);

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	icon_ = asset.LoadSprite("Resources/Images/Items/Coin.png");
	icon_->SetSize({ iconSize_, iconSize_ });
	icon_->SetPivot({ 0.5f,0.5f });

	// 数字描画
	drawNum_ = std::make_unique<DrawNumbers>(5);

	// 現在所持金
	moneyEase_.current = wallet_.GetCurrentMoney();
}

void MoneyUI::Shake(Vector2& drawPos) {
	// 所持金不足時の揺れタイマー
	if (shake_.timer.IsActive()) {
		shake_.timer.Update();

		float offset = sinf(shake_.timer.GetRemaining() * shake_.frequency) * shake_.amplitude;
		drawPos.x += offset;

		color_ = { 1,0,0,1 };
	} else {
		color_ = { 1,1,1,1 };
	}
}

void MoneyUI::Ease() {
	if (moneyEase_.timer.IsActive()) {
		moneyEase_.timer.Update();

		float t = 1.0f - (moneyEase_.timer.GetRemaining() / moneyEase_.duration);
		t = 1.0f - (1.0f - t) * (1.0f - t);
		moneyEase_.current = moneyEase_.start + int((moneyEase_.end - moneyEase_.start) * t);
	}
}

void MoneyUI::StartEase(int end) {
	moneyEase_.timer.Start(moneyEase_.duration);
	moneyEase_.start = moneyEase_.current;
	moneyEase_.end = end;
}

void MoneyUI::StartShake() {
	shake_.timer.Start(shake_.duration);
}

void MoneyUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	Vector2 drawPos = pos_;
	Shake(drawPos);
	Ease();

	// アイコン
	icon_->SetPosition({ drawPos.x, drawPos.y });
	render.DrawSprite(icon_.get());

	// 数字
	drawNum_->Draw(moneyEase_.current, numSpacing_, drawPos + Vector2{digitSize_, 0}, {digitSize_, digitSize_}, false, color_);
}
