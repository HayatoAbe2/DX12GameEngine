#pragma once
#include "GameCommon.h"
#include <Objects/Weapon/ShooterInfo/ShooterInfo.h>

/// <summary>
/// 当たり判定の足元表示
/// </summary>
class HitboxDisplay {
public:
	HitboxDisplay(Team team);
	void Update(float radius, Vector2 pos);
	void Draw();

private:
	// 表示する図形(円)
	std::unique_ptr<Primitive> primitive_;

	// 表示する高さ
	const float kHeight = 0.05f;
};

