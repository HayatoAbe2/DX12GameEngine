#pragma once

/// <summary>
/// 弾などの所属
/// </summary>
enum class Team {
	Player,
	Enemy
};

// 発射者の情報
struct ShooterInfo {
	Team team;
};