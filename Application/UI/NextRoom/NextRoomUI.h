#pragma once
#include "GameCommon.h"

class FloorManager;

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

