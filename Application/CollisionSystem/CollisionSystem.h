#pragma once
#include <memory>
#include "CollisionChecker/CollisionChecker.h"

class Player;
class Camera;
class BulletManager;
class EffectManager;

class CollisionSystem {
public:
	CollisionSystem(EffectManager* effectManager, BulletManager* bulletManager, EnemyManager* enemyManager, Camera* camera);
	void Update(Player* player);

private:
	// 弾マネージャ
	BulletManager* bulletManager_;
	// 敵マネージャ
	EnemyManager* enemyManager_;

	// 当たり判定
	std::unique_ptr<CollisionChecker> collisionChecker_ = nullptr;
};

