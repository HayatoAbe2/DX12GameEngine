#include "Wallet.h"
#include "UI/Money/MoneyUI.h"
#include <algorithm>
#include <Windows.h>

void Wallet::Initialize(MoneyUI* ui) {
	ui_ = ui;
}

void Wallet::Add(int amount) {
	money_ = min(money_ + amount, kMaxMoney);

	if (ui_) {
		ui_->StartEase(money_);
	}
}

bool Wallet::Pay(int amount) {
	if (money_ < amount) {
		if (ui_) {
			ui_->StartShake();
		}

		// 払えない
		return false;
	}

	money_ -= amount;

	if (ui_) {
		ui_->StartEase(money_);
	}
	return true;
}
