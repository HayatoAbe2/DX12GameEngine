#pragma once
#include "Enemy/Enemy.h"
class EnemyIdle : public EnemyState {
public:
	EnemyIdle();
	void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) override;

private:
	bool isMoving_ = false;
	std::unique_ptr<Timer> randomTimer_ = nullptr;

	const float minRandomMoveTime_ = 0.5f;
	const float maxRandomMoveTime_ = 2.0f;

	const float minRandomStopTime_ = 0.5f;
	const float maxRandomStopTime_ = 2.5f;
};

