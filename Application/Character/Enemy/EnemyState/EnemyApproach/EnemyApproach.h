#pragma once
#include "Character/Enemy/Enemy.h"
class EnemyApproach : public EnemyState{
public:
	EnemyApproach();
	void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) override;

private:
	std::unique_ptr<Timer> rotateTimer_;
	const float minRotateTimer_ = 1.0f / 20.0f;
	const float maxRotateTimer_ = 1.0f / 3.0f;
};

