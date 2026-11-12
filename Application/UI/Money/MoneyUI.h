#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"
#include <UI/DrawNumbers/DrawNumbers.h>

class Wallet;

/// <summary>
/// 所持金表示
/// </summary>
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

	Vector2 pos_ = {350, 20};
	Vector4 color_ = { 1,1,1,1 };
	float iconSize_ = 32.0f;
	float numSpacing_ = 27.0f;
	float digitSize_ = 32.0f;

	// オフセット
	const float valueOffsetY_ = 15.0f;

	// アイコン
	std::unique_ptr<Sprite> icon_;

	// 数字
	std::unique_ptr<DrawNumbers> drawNum_;

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

