#include "CollisionChecker.h"
#include "GameCommon.h"
#include "Player/Player.h"
#include "Enemy/Enemy.h"
#include "Bullet/Bullet.h"
#include "Effect/EffectManager.h"
#include <Bullet/WaveBullet.h>
#include <Bullet/FireBullet.h>
#include <Engine/Math/CollisionShape/Circle/Circle.h>

void CollisionChecker::Initialize(EffectManager* effectManager) {
	effectManager_ = effectManager;
}

void CollisionChecker::Check(Player* player, Bullet* bullet, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 敵の弾でなかったらor無敵時間なら判定しない
	if (!bullet->IsEnemyBullet() || bullet->IsDead() || player->IsBoosting()) { return; }

	if (Length(player->GetTransform().translate - bullet->GetTransform().translate) <=
		player->GetRadius() + bullet->GetTransform().scale.x / 2.0f) {
		player->Hit(bullet->GetDamage(), bullet->GetPrePos());
		bullet->Hit();
		camera->StartShake(1.0f, 3);
		if (dynamic_cast<FireBullet*>(bullet)) {
			effectManager_->SpawnExplodeEffect(bullet->GetTransform().translate);
		} else {
			effectManager_->SpawnHitEffect(bullet->GetTransform().translate);
		}

		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);
	}
}

void CollisionChecker::Check(Enemy* enemy, Bullet* bullet, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 敵の弾だったら判定しない
	if (bullet->IsEnemyBullet() || bullet->IsDead() || !bullet->CanHit()) { return; }

	if (Length(enemy->GetTransform().translate - bullet->GetTransform().translate) <=
		enemy->GetRadius() + bullet->GetTransform().scale.x / 2.0f) {
		enemy->Hit(bullet->GetDamage(), bullet->GetPrePos(), bullet->GetKnockback());
		if (dynamic_cast<WaveBullet*>(bullet)) {
			enemy->Slow();
		}
		bullet->Hit();
		camera->StartShake(1.0f, 3);
		if (dynamic_cast<FireBullet*>(bullet)) {
			effectManager_->SpawnExplodeEffect(bullet->GetTransform().translate);
		} else {
			effectManager_->SpawnHitEffect(bullet->GetTransform().translate);
		}
		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);
	}
}

void CollisionChecker::Check(Player* player, Enemy* enemy, Camera* camera) {
	Circle p = { player->GetRadius(), {player->GetTransform().translate.x, player->GetTransform().translate.z} };
	Circle e = { enemy->GetRadius(), {enemy->GetTransform().translate.x, enemy->GetTransform().translate.z} };
	if (p.CheckCollision(e)) {
		player->Hit(3.0f, enemy->GetTransform().translate);
	}
}