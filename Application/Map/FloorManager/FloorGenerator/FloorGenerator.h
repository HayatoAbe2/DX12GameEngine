#pragma once
#include "GameCommon.h"
#include "Map/Room/RoomData.h"
#include "Map/FloorConfig.h"

/// <summary>
/// フロアをロードして生成
/// </summary>
class FloorGenerator {
public:
	/// <summary>
	/// フロア生成
	/// </summary>
	/// <returns>最初の部屋</returns>
	Room* GenerateFloor(const FloorConfig& config);

	// 部屋の読み込み
	void LoadRoom(Room* room, Direction exitDir);

	// 逆方向
	Direction GetOpposite(Direction direction) { return opposite.at(direction); }

private:
	// 部屋生成
	void GenerateRooms(Room& room, int depth, const FloorConfig& config);

	// 部屋タイプの決定
	RoomType SelectRoomType();

	Room floor_;

	// 出口方向の割り当て
	std::unordered_map<std::string, Direction> exitDir{
		{ "westConnector", Direction::West },
		{ "eastConnector", Direction::East },
		{ "northConnector", Direction::North },
		{ "southConnector", Direction::South }
	};

	// 逆方向
	std::unordered_map<Direction, Direction> opposite{
		{ Direction::North, Direction::South },
		{ Direction::East, Direction::West },
		{ Direction::South, Direction::North },
		{ Direction::West, Direction::East }
	};
};

