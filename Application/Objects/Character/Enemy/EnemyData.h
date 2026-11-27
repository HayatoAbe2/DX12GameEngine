#pragma once
#include <optional>
#include <string>
#include <vector>
#include "EnemyStatus.h"
#include "Objects/Character/CharacterAnimation.h"
#include <array>

/// <summary>
/// 武器を切り替えるまでの攻撃回数
/// </summary>
struct WeaponSwitch {
	int minAttacks = 1;
	int maxAttacks = 1;
};

/// <summary>
/// 敵が使うアニメーション
/// </summary>
enum class EnemyAnimations {
	Idle,
	Attack,
	Dead,
	Hit,

	Count
};

/// <summary>
/// 敵データ
/// </summary>
struct EnemyData {
	// モデル
	std::string modelName;
	// アニメーション
	std::array<CharacterAnimation,static_cast<int>(EnemyAnimations::Count)> animations;

	// ステータス
	EnemyStatus status;

	// 武器
	std::vector<std::string> weapons;
	WeaponSwitch weaponSwitch;   // 武器切り替えまでの回数

	// 一度の出現数幅
	int minSpawnCount = 1;
	int maxSpawnCount = 1;
};
