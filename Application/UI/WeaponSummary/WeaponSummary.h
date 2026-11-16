#pragma once
#include "GameCommon.h"
#include <Objects/Weapon/Weapon.h>
#include <UI/DrawNumbers/DrawNumbers.h>

constexpr int kAtkDigitCount = 4;
constexpr int kBulletDigitCount = 3;
constexpr int kReloadDigitCountL = 3;
constexpr int kReloadDigitCountR = 4;

/// <summary>
/// 武器の基本性能を表示
/// </summary>
class WeaponSummaryUI {
public:
	WeaponSummaryUI();
	void Update(const WeaponData& data); // 該当物ありのとき
	void Update(); // 該当物なしのとき
	void Draw();

private:
	std::unique_ptr<Sprite> summaryWindow_; // 枠
	std::unique_ptr<Sprite> summaryItems_; // 項目
	std::unique_ptr<DrawNumbers> atkNums_; // 攻撃力
	std::unique_ptr<DrawNumbers> maxBulletNums_; // 最大弾数
	std::unique_ptr<DrawNumbers> reloadSpeedNumsL_; // リロード速度(秒)
	std::unique_ptr<DrawNumbers> reloadSpeedNumsR_; // リロード速度(小数点以下)

	bool isActive_ = false;
};

