#include "EnemyApproach.h"
#include "Enemy/EnemyState/EnemyAttack/EnemyAttack.h"
#include "Player/Player.h"

EnemyApproach::EnemyApproach() {
	rotateTimer_ = std::make_unique<Timer>();
}

void EnemyApproach::Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();

	if (rotateTimer_->IsFinished()) {
		// 方向転換の間隔
		rotateTimer_->Start(ctx.RandomFloat(minRotateTimer_, maxRotateTimer_));

		float length = Length(player->GetTransform().translate - enemy->GetTransform().translate);

		EnemyStatus status = enemy->GetStatus();
		if (length > status.attackRadius) {
			// プレイヤー方向に移動
			Vector3 targetDir = Normalize(player->GetTransform().translate - enemy->GetTransform().translate);
			velocity_ = Vector3{ targetDir.x,0,targetDir.z } * status.moveSpeed;

			if (velocity_.x != 0 || velocity_.z != 0) {
				Transform transform = enemy->GetTransform();
				transform.rotate = { 0,-std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f,0 }; 
				enemy->SetTransform(transform);
			}
		} else {
			enemy->SetState(std::move(std::make_unique<EnemyAttack>()));
			return;
		}
	}

	rotateTimer_->Update();
}
