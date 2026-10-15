#pragma once
#include "GameCommon.h"

enum class RoomType {
	Start,
	Combat,
	Boss,
	Shop,
	Event,
	Goal,

	Count
};

enum class Direction {
	North,
	East,
	South,
	West
};

struct Room;
struct RoomConnector {
	AABB2D collider;
	Direction direction;
	Room* connectedRoom = nullptr;
};

struct Room {
	RoomType type;
	std::vector<Room> nextRooms;
	std::vector<RoomConnector> connector;
	Direction startDirection;
	Vector2 startPos;
};

// 抽選範囲の数
constexpr unsigned int roomNumberRange[6] = {1,7,1,1,1,1};