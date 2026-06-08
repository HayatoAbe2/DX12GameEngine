#include "EnemyAttack.h"
#include "Player/Player.h"

EnemyAttack::EnemyAttack() {
	rotateTimer_ = std::make_unique<Timer>();
}

void EnemyAttack::Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();
	Transform transform = enemy->GetTransform();
	Timer* attackTimer = enemy->GetAttackTimer();

	// 攻撃の向き
	Vector3 attackDirection = Normalize(player->GetTransform().translate - transform.translate);
	transform.rotate = { 0,-std::atan2(attackDirection.z, attackDirection.x) + float(std::numbers::pi) / 2.0f,0 };

	float remain = attackTimer->GetRemaining();
	if (remain <= attackMotionStart_) {
		// 攻撃前警告(スケール変更)
		float sizeEase;
		if (remain < attackMotionStart_ / 2) {
			float t = (1 - float(attackMotionStart_ - remain) / float(attackMotionStart_)) * 2;
			sizeEase = EaseIn(1, 1.7f, t);
		} else {
			float t = float(attackMotionStart_ - remain) / float(attackMotionStart_) * 2;
			sizeEase = EaseIn(1, 1.7f, t);
		}
		transform.scale = { sizeEase,sizeEase,sizeEase };

		Vector3 direction = Normalize(velocity_);
		EnemyStatus status = enemy->GetStatus();
		velocity_.x = direction.x * status.moveSpeed * (1.0f - enemy->GetCurrentWeapon()->GetData().weight);
		velocity_.z = direction.z * status.moveSpeed * (1.0f - enemy->GetCurrentWeapon()->GetData().weight);
	} else {
		if (rotateTimer_->IsFinished()) {
			// 方向転換の間隔
			rotateTimer_->Start(ctx.RandomFloat(minRotateTimer_, maxRotateTimer_));

			// 移動
			Vector2 direction = Normalize(Vector2{ ctx.RandomFloat(-1,1), ctx.RandomFloat(-1,1) });
			EnemyStatus status = enemy->GetStatus();
			velocity_.x = direction.x * status.moveSpeed;
			velocity_.z = direction.y * status.moveSpeed;
		}

		rotateTimer_->Update();
	}

	if (attackTimer->IsFinished() || enemy->IsAttacking()) {
		enemy->Attack(bulletManager, attackDirection, nullptr);
	} else {
		attackTimer->Update();
	}

	enemy->SetTransform(transform);
}

// EaseInBackの数値調整版
float EnemyAttack::EaseIn(float start, float end, float t) {
	if (t > 1.0f)t = 1.0f;
	else if (t < 0.0f)t = 0.0f;

	float easedT = 1.0f - cosf((t * float(std::numbers::pi)) / 2.0f);
	return (1.0f - easedT) * start + easedT * end;
}