#pragma once
#include "GameCommon.h"
#include "Map/Room/RoomData.h"

class FloorManager;

/// <summary>
/// 次の部屋説明
/// </summary>
class NextRoomUI {
public:
	NextRoomUI(FloorManager* floorManager);

	void Update();
	void Draw();

private:

	const float iconSize_ = 80.0f;
	const float rewardIconSize_ = 60.0f;
	Vector2 rewardOffset_ = { 0,65 };
	RoomState roomState_;

	// アイコン
	std::vector<std::unique_ptr<Sprite>> icons_;
	// 報酬アイコン
	std::vector<std::unique_ptr<Sprite>> rewardIcons_;

	// 部屋マネージャ
	FloorManager* floorManager_;
};

