#include "CollisionChecker.h"
#include "GameCommon.h"
#include "Character/Player/Player.h"
#include "Character/Enemy/Enemy.h"
#include "Bullet/Bullet.h"
#include "Effect/EffectManager.h"
#include <Bullet/WaveBullet.h>
#include <Bullet/FireBullet.h>
#include <Bullet/OrbitBullet.h>

void CollisionChecker::Initialize(EffectManager* effectManager) {
	effectManager_ = effectManager;
}

void CollisionChecker::Check(Player* player, Bullet* bullet, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// 敵の弾でなかったらor無敵時間なら判定しない
	if (!bullet->IsEnemyBullet() ||
		bullet->IsDead() ||
		player->IsBoosting()) {
		return;
	}

	Segment2D segment = { bullet->GetPrePos(), ToXZ(bullet->GetTransform().translate) };
	Circle circle = { player->GetRadius() + bullet->GetTransform().scale.x / 2.0f, ToXZ(player->GetTransform().translate) };
	if (CheckCollision(segment, circle)) {
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

	Segment2D segment = { bullet->GetPrePos(), ToXZ(bullet->GetTransform().translate) };
	Circle circle = { enemy->GetRadius() + bullet->GetTransform().scale.x / 2.0f, ToXZ(enemy->GetTransform().translate) };
	if (CheckCollision(segment, circle)) {
		enemy->Hit(bullet->GetDamage(), bullet->GetPrePos(), bullet->GetKnockback());
		if (dynamic_cast<WaveBullet*>(bullet)) { 
			enemy->Slow();
			bullet->Hit();
		} else if (dynamic_cast<OrbitBullet*>(bullet)) {
		} else {
			bullet->Hit();
		}
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
	if (CheckCollision(p, e)) {
		player->Hit(3.0f, ToXZ(enemy->GetTransform().translate));
	}
}

