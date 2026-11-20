#pragma once
#include "Objects/Character/Enemy/Enemy.h"

/// <summary>
/// 攻撃範囲までの接近
/// </summary>
class EnemyApproach : public EnemyActionState{
public:
	void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) override;

private:
	Timer rotateTimer_;
	const float minRotateTimer_ = 1.0f / 20.0f;
	const float maxRotateTimer_ = 1.0f / 3.0f;
};

