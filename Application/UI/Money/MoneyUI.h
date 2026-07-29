#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

class Wallet;

class MoneyUI {
public:
	MoneyUI(Wallet& wallet);

	// 増減
	void StartEase(int end);
	// 不足
	void StartShake();

	void Draw();

private:
	void Shake(Vector2& drawPos);
	void Ease();

	Vector2 pos_ = {325, 20};
	float numSpacing_ = 27.0f;
	float digitSize_ = 32.0f;
	float iconSize_ = 32.0f;
	// 画像上の数字サイズ
	float digitWidth_ = 64.0f;

	// オフセット
	float valueOffsetY_ = 15.0f;

	// アイコン
	std::unique_ptr<Sprite> icon_;

	// 数値
	std::array<std::unique_ptr<Sprite>, 4> numbers_;

	// 追跡するWallet
	Wallet& wallet_;

	struct MoneyEase {
		int start = 0;
		int end = 0;
		int current = 0;
		float t = 0;

		Timer timer;
		float duration = 0.4f;
	};
	MoneyEase moneyEase_;

	struct ShakeParam {
		Timer timer;
		float duration = 0.3f;
		float amplitude = 10;
		float frequency = 20;
	};
	ShakeParam shake_;
};

