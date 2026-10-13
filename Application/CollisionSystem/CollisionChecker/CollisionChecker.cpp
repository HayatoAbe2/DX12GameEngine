#include "CollisionChecker.h"
#include "GameCommon.h"
#include "Character/Player/Player.h"
#include "Character/Enemy/Enemy.h"
#include "Bullet/Bullet.h"
#include "Effect/EffectManager.h"
#include "Map/MapCheck.h"

void CollisionChecker::Initialize(EffectManager* effectManager, Camera* camera) {
	effectManager_ = effectManager;
	camera_ = camera;
}

void CollisionChecker::CheckPlayer(Player* player, Bullet* bullet) {
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
		if (player->IsBoosting()) {
			// 回避
			player->OnDodge();
		} else {
			// プレイヤーにヒット
			player->Hit(bullet->GetDamage(), bullet->GetPrePos());
			bullet->Hit();
			bullet->OnHitAnything(effectManager_);

			camera_->StartShake(1.0f, 3);
			if (bullet->GetData().traits.onHitAnything.explode) {
				effectManager_->SpawnExplodeEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
			} else {
				effectManager_->SpawnHitEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
			}

			audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);
			player->OnHit(bullet->GetPrePos());
		}
	}
}

void CollisionChecker::CheckEnemy(Enemy* enemy, Bullet* bullet, Player* player) {
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

		camera_->StartShake(0.5f, 2);
		if (bullet->GetData().traits.onHitAnything.explode) {
			effectManager_->SpawnExplodeEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		} else {
			effectManager_->SpawnHitEffect({ bullet->GetCollider().center.x, 0.5f, bullet->GetCollider().center.y });
		}

		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);

		// ダメージを与えた時の効果
		player->OnDealDamage(circle.center);
		if (enemy->IsDead()) {
			// 撃破時効果
			player->OnEliminate(circle.center);
		}
	}
}

void CollisionChecker::CheckContactDamage(Player* player, Enemy* enemy) {
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

		camera_->StartShake(0.5f, 2);
		effectManager_->SpawnHitEffect(player->GetTransform().translate);
		audio.SoundPlay(L"Resources/Sounds/SE/hit.mp3", false);
	}
}

void CollisionChecker::CheckPush(Character* characterA, Character* characterB, MapCheck* mapCheck) {
	Circle circleA = { characterA->GetRadius(), ToXZ(characterA->GetTransform().translate) };
	Circle circleB = { characterB->GetRadius(), ToXZ(characterB->GetTransform().translate) };

	// 衝突していなかったら終了
	if (!CheckCollision(circleA, circleB)) { return; }
	
	Vector2 distance = circleA.center - circleB.center;
	float length = Length(distance);
	if (length > 0.001f) {
		// 同じ位置の場合
		distance = { 1,0 };
	}

	// 重なり量
	float penetration = circleA.radius + circleB.radius - length;

	// 押す
	Vector2 push = Normalize(distance);

	float weightA = characterA->GetWeight() / (characterA->GetWeight() + characterB->GetWeight());
	Vector2 pushedA = circleA.center + push * weightA;
	Vector2 pushedB = circleB.center - push * (1.0f - weightA);

	// マップとの判定で修正
	mapCheck->ResolveCollisionX(pushedA, circleA.radius, characterA->CanFly());
	mapCheck->ResolveCollisionY(pushedA, circleA.radius, characterA->CanFly());
	mapCheck->ResolveCollisionX(pushedB, circleB.radius, characterB->CanFly());
	mapCheck->ResolveCollisionY(pushedB, circleB.radius, characterB->CanFly());

	// 適用
	characterA->SetPosition(pushedA);
	characterB->SetPosition(pushedB);
}