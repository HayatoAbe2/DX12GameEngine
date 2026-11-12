#pragma once
#include "GameCommon.h"

/// <summary>
/// 数字画像を並べて描画する
/// </summary>
class DrawNumbers {
public:
	// 詰める位置
	enum class Alignment {
		Left,
		Center
	};

	DrawNumbers(int maxDigits);

	// 描画
	void Draw(
		int value, // 表示する数値
		float digitSpace, // 桁間の空白
		Vector2 position, // 左詰めなら左端、中央揃えなら中央
		Vector2 size,
		bool drawZero = false, // 最大桁数まで使わないときに0をつけるか
		Vector4 color = { 1,1,1,1 },
		Alignment alignment = Alignment::Left // 詰める位置
	);

private:
	// 数字画像
	std::vector<std::unique_ptr<Sprite>> sprites_;

	// 画像上の数字サイズ
	const float kDigitWidth = 64.0f;
};

