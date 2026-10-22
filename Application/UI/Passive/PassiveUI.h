#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

class Player;

class PassiveUI {
public:
	PassiveUI(Player* player);

	void Draw();
	
private:
	// プレイヤー
	Player* player_ = nullptr;

	const Vector2 kExplainOffset = { 0,75 };

	int select_ = 0;
	bool isSelectShow_ = false;
	Timer padSelectTimer_;
	float kPadSelectTime = 3.0f;
};

