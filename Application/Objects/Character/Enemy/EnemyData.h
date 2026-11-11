#pragma once
#include <optional>
#include <string>
#include <vector>
#include "EnemyStatus.h"

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

	// 一度の出現数幅
	int minSpawnCount = 1;
	int maxSpawnCount = 1;
};
