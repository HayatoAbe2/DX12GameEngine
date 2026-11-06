#pragma once
#include "GameCommon.h"

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

	// アイコン
	std::vector<std::unique_ptr<Sprite>> icons_;

	// 部屋マネージャ
	FloorManager* floorManager_;
};

