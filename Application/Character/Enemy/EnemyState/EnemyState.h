#pragma once
#include "GameCommon.h"

class Enemy;
class Player;
class MapCheck;
class BulletManager;

// 行動状態
class EnemyState {
public:
	virtual void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) = 0;
	virtual Vector3 GetVelocity() { return velocity_; }

protected:
	Vector3 velocity_{};
};