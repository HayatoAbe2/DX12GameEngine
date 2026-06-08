#pragma once
#include "Enemy/Enemy.h"
class EnemyAttack : public EnemyState {
public:
	EnemyAttack();
	void Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) override;

private:
	// 攻撃モーション
	float attackMotionStart_ = 0.5f;
	float EaseIn(float start, float end, float t);

	// 方向転換
	std::unique_ptr<Timer> rotateTimer_;
	const float minRotateTimer_ = 0.5f;
	const float maxRotateTimer_ = 0.8f;
};


