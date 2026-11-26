#pragma once
#include <optional>
#include <string>
#include <vector>
#include "EnemyStatus.h"

/// <summary>
/// 武器を切り替えるまでの攻撃回数
/// </summary>
struct WeaponSwitch {
	int minAttacks = 1;
	int maxAttacks = 1;
};

/// <summary>
/// 敵データ
/// </summary>
struct EnemyData {
	// モデル
	std::string modelName;
	int animationIndex = -1;

	// ステータス
	EnemyStatus status;

	// 武器
	std::vector<std::string> weapons;
	WeaponSwitch weaponSwitch;   // 武器切り替えまでの回数

	// 一度の出現数幅
	int minSpawnCount = 1;
	int maxSpawnCount = 1;
};
