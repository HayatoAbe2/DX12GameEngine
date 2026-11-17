#pragma once
#include "GameCommon.h"
#include "Rarity.h"

// 報酬
enum class RewardType { None, Money, Weapon, Passive };

struct RewardPlan {
	RewardType type = RewardType::None;
	Rarity rarity = Rarity::Common;
	int amount = 0; // Money用
};

// 部屋の予定(入室前に確定)
struct RoomPlan {
	bool planned = false;
	std::vector<std::string> enemies; // 出現する敵の名前
	RewardPlan reward;
};

// 部屋の状態
enum class RoomState {
	Entered,	// 入った直後
	InCombat,	// 戦闘中
	Cleared		// 敵を倒した
};

// 部屋タイプ
enum class RoomType {
	Start,
	Combat,
	Elite,
	Boss,
	Shop,
	Event,
	Goal,

	Count
};

// 方角
enum class Direction {
	North,
	East,
	South,
	West
};

struct Room;
// 出入口
struct RoomConnector {
	AABB2D collider;
	Direction direction;
	Room* connectedRoom = nullptr;
};

// 部屋構造体
struct Room {
	RoomType type;
	RoomPlan plan;
	RoomState state = RoomState::Entered;
	std::vector<Room> nextRooms;
	std::vector<RoomConnector> connector;
	Direction startDirection;
	Vector2 startPos;
};

// 抽選範囲の数
constexpr unsigned int roomNumberRange[7] = {1,7,1,1,1,1,1};