#pragma once
#include "GameCommon.h"
#include <Objects/Weapon/Weapon.h>
#include <UI/DrawNumbers/DrawNumbers.h>

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
	std::unique_ptr<Sprite> background_;

	bool isActive_ = false;
};

