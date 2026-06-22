#include "EnemyIdle.h"
#include "Character/Player/Player.h"
#include "Map/MapCheck.h"
#include "Character/Enemy/EnemyState/EnemyApproach/EnemyApproach.h"

EnemyIdle::EnemyIdle() {
	randomTimer_ = std::make_unique<Timer>();
}

void EnemyIdle::Update(Enemy* enemy, Player* player, MapCheck* mapCheck, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();

	if (Length(player->GetTransform().translate - enemy->GetTransform().translate) < enemy->GetStatus().defaultSearchRadius) {
		if (mapCheck->EnemyCanSeePlayer(enemy->GetTransform().translate, player->GetTransform().translate)) {
			// 発見
			enemy->SetState(std::move(std::make_unique<EnemyApproach>()));
			return;
		}
	}

	// プレイヤーを見つけてない
	if (isMoving_) {
		randomTimer_->Update();
		if (randomTimer_->IsFinished()) {
			isMoving_ = false;
			randomTimer_->Start(ctx.RandomFloat(minRandomStopTime_, maxRandomStopTime_));
			velocity_ = {};
		}
	} else {
		randomTimer_->Update();
		if (randomTimer_->IsFinished()) {
			isMoving_ = true;
			randomTimer_->Start(ctx.RandomFloat(minRandomMoveTime_, maxRandomMoveTime_));

			Vector2 direction = Normalize(Vector2{ ctx.RandomFloat(-1,1), ctx.RandomFloat(-1,1) });
			velocity_.x = direction.x * enemy->GetStatus().moveSpeed / 3.0f;
			velocity_.z = direction.y * enemy->GetStatus().moveSpeed / 3.0f;

			if (velocity_.x != 0 || velocity_.z != 0) {
				Transform transform = enemy->GetTransform();
				transform.rotate = { 0,-std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f,0 };
				enemy->SetTransform(transform);
			}
		}
	}
}
