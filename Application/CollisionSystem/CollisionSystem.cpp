#include "CollisionSystem.h"
#include "Bullet/BulletManager.h"
#include "Character/Player/Player.h"
#include "Character/Enemy/EnemyManager/EnemyManager.h"
#include "Character/Enemy/Enemies/Spiker.h"
#include "Map/MapCheck.h"

CollisionSystem::CollisionSystem(EffectManager* effectManager, BulletManager* bulletManager, EnemyManager* enemyManager, Camera* camera) {
	collisionChecker_ = std::make_unique<CollisionChecker>();
	collisionChecker_->Initialize(effectManager, camera);

	bulletManager_ = bulletManager;
	enemyManager_ = enemyManager;
}

void CollisionSystem::Update(Player* player, MapCheck* mapCheck) {
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
			collisionChecker_->CheckContactDamage(player, enemy);
		}
	}

	// キャラクター同士の押し出し処理
	std::vector<Character*> characters;
	for (auto enemy : enemyManager_->GetEnemies()) {
		characters.push_back(enemy);
	}
	characters.push_back(player);

	int kIterationCount = 4; // 反復回数
	for (int iteration = 0; iteration < kIterationCount; ++iteration) {
		// 全キャラクターの組み合わせを調べる
		for (size_t i = 0; i < characters.size(); ++i) {
			Character* characterA = characters[i];
			if (!characterA || characterA->IsDead() || characterA->IsInvincible()) {
				continue;
			}

			// Aより後ろのキャラクターだけを調べる
			for (size_t j = i + 1; j < characters.size(); ++j) {
				Character* characterB = characters[j];
				if (!characterB || characterB->IsDead() || characterB->IsInvincible() ||
					(characterA->CanFly() != characterB->CanFly())) { // 地上と空中では当たらない
					continue;
				}

				collisionChecker_->CheckPush(characterA, characterB, mapCheck);
			}
		}
	}
}