#pragma once
#include "GameCommon.h"

class Wallet;

class MoneyUI {
public:
	MoneyUI(Wallet* wallet);

	void Draw();

private:
	// アイコン
	std::unique_ptr<Sprite> icon_;

	// 数値
	std::array<std::unique_ptr<Sprite>, 4> amount_;

	// 追跡するWallet
	Wallet* wallet_ = nullptr;
};

