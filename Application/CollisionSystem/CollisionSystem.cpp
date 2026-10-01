#include "CollisionSystem.h"
#include "Bullet/BulletManager.h"
#include "Character/Enemy/EnemyManager.h"
#include "Character/Enemy/Enemies/Spiker.h"

CollisionSystem::CollisionSystem(EffectManager* effectManager, BulletManager* bulletManager, EnemyManager* enemyManager, Camera* camera) {
	collisionChecker_ = std::make_unique<CollisionChecker>();
	collisionChecker_->Initialize(effectManager, camera);

	bulletManager_ = bulletManager;
	enemyManager_ = enemyManager;
}

void CollisionSystem::Update(Player* player) {
	for (const auto& bullet : bulletManager_->GetBullets()) {
		// プレイヤーと敵弾
		collisionChecker_->CheckPlayer(player, bullet);

		for (auto enemy : enemyManager_->GetEnemies()) {
			// 敵と自弾
			collisionChecker_->CheckEnemy(enemy, bullet, player);
		}
	}

	for (auto enemy : enemyManager_->GetEnemies()) {
		if (dynamic_cast<Spiker*>(enemy)) {
			// 敵とプレイヤーの接触
			collisionChecker_->CheckContact(player, enemy);
		}
	}

}
