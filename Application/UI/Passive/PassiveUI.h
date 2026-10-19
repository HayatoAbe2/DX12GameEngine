#pragma once
#include "GameCommon.h"

class Player;

class PassiveUI {
public:
	PassiveUI(Player* player);

	void Draw();
	
private:
	// プレイヤー
	Player* player_ = nullptr;

	const Vector2 kExplainOffset = { 0,75 };
};

