#pragma once
class MoneyUI;

class Wallet {
public:
	void Initialize(MoneyUI* ui);

	// 追加
	void Add(int amount);

	// 払う
	bool Pay(int amount);

 	int GetCurrentMoney() { return money_; }

private:
	static constexpr int kMaxMoney = 9999;

	int money_ = 300;

	MoneyUI* ui_;
};

