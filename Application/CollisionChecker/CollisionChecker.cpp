#include "CollisionChecker.h"
#include "GameCommon.h"
#include "Character/Player/Player.h"
#include "Character/Enemy/Enemy.h"
#include "Bullet/Bullet.h"
#include "Effect/EffectManager.h"

void CollisionChecker::Initialize(EffectManager* effectManager) {
	effectManager_ = effectManager;
}

void CollisionChecker::Check(Player* player, Bullet* bullet, Camera* camera, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 敵の弾でなかったらor無敵時間なら判定しない
	if (!bullet->IsEnemyBullet() ||
		player->IsInvincible()) {
		return;
	}

	if (bullet->IsDead()) return;

	Segment2D segment = { bullet->GetPrePos(), bullet->GetCollider().center };
	Circle circle = { player->GetRadius() + bullet->GetCollider().radius, ToXZ(player->GetTransform().translate) };
	if (CheckCollision(segment, circle)) {
		player->Hit(bullet->GetDamage(), bullet->GetPrePos());

		bullet->Hit();
		bullet->OnHitAnything(effectManager_);

		camera->StartShake(1.0f, 3);
		if (bullet->GetData().traits.onHitAnything.explode) {
			effectManager_->SpawnExplodeEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		} else {
			effectManager_->SpawnHitEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		}

		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);

		player->OnHit(bullet->GetPrePos(), bulletManager);
	}
}

void CollisionChecker::Check(Enemy* enemy, Bullet* bullet, Camera* camera, Player* player, EnemyManager* enemyManager) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 敵の弾だったら判定しない
	if (bullet->IsEnemyBullet() || bullet->IsDead() || !bullet->CanHit()) { return; }

	Segment2D segment = { bullet->GetPrePos(), bullet->GetCollider().center };
	Circle circle = { enemy->GetRadius() + bullet->GetCollider().radius, ToXZ(enemy->GetTransform().translate) };

	if (CheckCollision(segment, circle)) {
		enemy->Hit(bullet->GetDamage(), bullet->GetPrePos(), bullet->GetKnockback());

		// 減速効果
		if (bullet->GetData().traits.onHitEnemy.slow) {
			enemy->Slow();
		}

		bullet->Hit();
		bullet->OnHitAnything(effectManager_);

		camera->StartShake(0.5f, 2);
		if (bullet->GetData().traits.onHitAnything.explode) {
			effectManager_->SpawnExplodeEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		} else {
			effectManager_->SpawnHitEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		}

		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);

		player->OnDealDamage(circle.center, enemyManager);
	}
}

void CollisionChecker::Check(Player* player, Enemy* enemy, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 判定しない条件
	if (player->IsInvincible()) {
		return;
	}

	Segment2D segment = {
		ToXZ(player->GetPrePos()),
		ToXZ(player->GetTransform().translate)
	};

	Circle circle = {
		player->GetRadius() + enemy->GetRadius(),
		ToXZ(enemy->GetTransform().translate)
	};

	if (CheckCollision(segment, circle)) {
		player->Hit(3.0f, ToXZ(enemy->GetPrePos()));

		camera->StartShake(0.5f, 2);
		effectManager_->SpawnHitEffect(player->GetTransform().translate);
		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);
	}
}

