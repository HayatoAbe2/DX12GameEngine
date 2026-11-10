#pragma once
#include "GameCommon.h"
#include <Objects/Weapon/Weapon.h>

/// <summary>
/// 武器の基本性能を表示
/// </summary>
class WeaponSummaryUI {
public:
	WeaponSummaryUI();
	void Update(const WeaponData& data);
	void Update(); // 該当物なしのとき
	void Draw();

private:
	std::unique_ptr<Sprite> background_;

	// 動画
	std::unique_ptr<VideoPlayer> video_;
	std::unique_ptr<Sprite> videoSprite_;

	bool isActive_ = false;
};

