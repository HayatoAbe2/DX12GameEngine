#pragma once

enum class Team {
	Player,
	Enemy
};

// 発射者の情報
struct ShooterInfo {
	Team team;
};