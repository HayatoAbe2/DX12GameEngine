#include "Wallet.h"
#include <algorithm>

void Wallet::Add(int amount) {
	money_ = std::min(money_ + amount, kMaxMoney);
}

bool Wallet::Pay(int amount) {
	if (money_ < amount) {
		// 払えない
		return false;
	}

	money_ -= amount;
	return true;
}
