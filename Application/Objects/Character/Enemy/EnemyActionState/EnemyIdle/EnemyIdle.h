#pragma once
#include "Objects/Character/Enemy/Enemy.h"

/// <summary>
/// 敵の待機状態
/// </summary>
class EnemyIdle : public EnemyActionState {
public:
	void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) override;

private:
	bool isMoving_ = false;
	Timer randomTimer_;

	const float minRandomMoveTime_ = 0.5f;
	const float maxRandomMoveTime_ = 2.0f;

	const float minRandomStopTime_ = 0.5f;
	const float maxRandomStopTime_ = 2.5f;
};

