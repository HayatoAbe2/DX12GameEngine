#include "DrawNumbers.h"

DrawNumbers::DrawNumbers(int maxDigits) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 桁数分の数字画像
	for (int i = 0; i < maxDigits; ++i) {
		std::unique_ptr<Sprite> num;
		num = asset.LoadSprite("Resources/Images/Text/Numbers.png");
		num->SetPivot({ 0.5f,0.5f });
		sprites_.push_back(std::move(num));
	}
}

void DrawNumbers::Draw(int value, float digitSpace, Vector2 position, Vector2 size, bool drawZero, Vector4 color, Alignment alignment) {
	// 桁数を取得
	int digitCount = 1;
	int temp = value;

	while (temp >= 10) {
		temp /= 10;
		digitCount++;
	}

	int maxDigits = static_cast<int>(sprites_.size());
	// 最大桁数を超えないようにする
	digitCount = min(digitCount, maxDigits);

	// 表示桁数
	int displayCount = drawZero ? maxDigits : digitCount;

	// 全体の幅
	float totalWidth = (displayCount - 1) * digitSpace + size.x;

	// 描画開始位置
	float startX = position.x;

	if (alignment == Alignment::Center) {
		startX -= totalWidth / 2.0f;
	}

	// 数字を取得
	std::vector<int> digits(displayCount, 0);

	for (int i = displayCount - 1; i >= 0; --i) {
		digits[i] = value % 10;
		value /= 10;
	}

	// 描画
	for (int i = 0; i < displayCount; ++i) {
		// 描画位置
		float x = startX + i * digitSpace;
		sprites_[i]->SetPosition({ x, position.y });
		sprites_[i]->SetSize(size);
		sprites_[i]->SetColor(color);
		sprites_[i]->SetTextureRect(
			digits[i] * kDigitWidth,
			0,
			kDigitWidth,
			kDigitWidth
		);

		auto& ctx = GameContext::GetInstance();
		auto& render = ctx.Render();
		render.DrawSprite(sprites_[i].get());
	}
}
