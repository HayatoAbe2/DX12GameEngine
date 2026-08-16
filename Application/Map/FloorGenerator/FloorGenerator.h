#pragma once
#include "GameCommon.h"
#include "Map/Room/RoomData.h"

class FloorGenerator {
public:
	/// <summary>
	/// フロア生成
	/// </summary>
	/// <returns>最初の部屋</returns>
	Room* GenerateFloor();

	// 部屋の読み込み
	void LoadRoom(Room* room, Direction exitDir);

	// 逆方向
	Direction GetOpposite(Direction direction);
private:
	// 部屋生成
	void GenerateRooms(Room& room, int depth, int maxDepth);

	// 部屋タイプの決定
	RoomType SelectRoomType();

	// 方向決め
	Direction GetDirection(float rotationY);

	Room floor_;
};

