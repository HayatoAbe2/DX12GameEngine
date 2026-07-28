#include "MoneyUI.h"
#include "Wallet/Wallet.h"

MoneyUI::MoneyUI(Wallet& wallet) : wallet_(wallet) {
	wallet_ = wallet;
    wallet_.Initialize(this);

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	icon_ = asset.LoadSprite("Resources/Items/Coin.png");
	icon_->SetSize({ iconSize_, iconSize_ });
	icon_->SetPivot({ 0.5f,0.5f });

	for (int i = 0; i < numbers_.size(); ++i) {
		numbers_[i] = asset.LoadSprite("Resources/Text/Numbers.png");
		numbers_[i]->SetSize({ digitSize_, digitSize_ });
		numbers_[i]->SetPivot({ 0.5f,0.5f });
	}

    moneyEase_.current = wallet_.GetCurrentMoney();
}

void MoneyUI::Shake(Vector2& drawPos) {
    // 所持金不足時の揺れタイマー
    if (shake_.timer.IsActive()) {
        shake_.timer.Update();

        float offset = sinf(shake_.timer.GetRemaining() * shake_.frequency) * shake_.amplitude;
        drawPos.x += offset;

        for (int i = 0; i < numbers_.size(); ++i) {
            numbers_[i]->SetColor({ 1,0,0,1 });
        }
    } else {
        for (int i = 0; i < numbers_.size(); ++i) {
            numbers_[i]->SetColor({ 1,1,1,1 });
        }
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

    int value = moneyEase_.current;
    // 表示桁数の決定
    int digitCount = 1;
    int temp = value;
    while (temp >= 10) {
        temp /= 10;
        digitCount++;
    }

    // アイコン位置
    icon_->SetPosition({ drawPos.x, drawPos.y });
    render.DrawSprite(icon_.get());

    // 数字開始位置
    float numberX = drawPos.x + numSpacing_;

    // 左詰めで並べる
    for (int i = 0; i < digitCount; ++i) {
        int digit = value % 10;
        value /= 10;

        numbers_[i]->SetPosition({numberX + numSpacing_ * (digitCount - 1 - i), drawPos.y});
        numbers_[i]->SetTextureRect(digitWidth_ * digit, 0, digitWidth_, digitWidth_);
        render.DrawSprite(numbers_[i].get());
    }
}
