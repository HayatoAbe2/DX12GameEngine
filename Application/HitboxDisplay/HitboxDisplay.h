#pragma once
#include "GameCommon.h"
#include <Objects/Weapon/ShooterInfo/ShooterInfo.h>

// 当たり判定の足元表示
class HitboxDisplay {
public:
	HitboxDisplay(Team team);
	void Update(float radius, Vector2 pos);
	void Draw();

private:
	std::unique_ptr<Primitive> primitive_;

	const float kHeight = 0.05f;
};

