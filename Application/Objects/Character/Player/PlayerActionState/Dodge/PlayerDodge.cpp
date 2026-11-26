#include "PlayerDodge.h"
#include "Objects/Character/Player/Player.h"
#include "Map/MapCheck.h"
#include <cmath>
#include <numbers>

PlayerDodge::PlayerDodge() {
	timer_.Start(duration_);
}

bool PlayerDodge::Update(Player* player, MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	Transform transform = player->GetTransform();
	direction_ = Normalize(player->GetVelocity());
	Vector3 velocity = direction_ * speed_ * ctx.GetDeltatime();
	Vector2 pos = { transform.translate.x, transform.translate.z };
	for (int i = 0; i < 4; ++i) {
		pos.x += velocity.x / 4.0f;
		mapCheck->ResolveCollisionX(pos, player->GetRadius(), true);
		pos.y += velocity.z / 4.0f;
		mapCheck->ResolveCollisionY(pos, player->GetRadius(), true);
	}
	transform.translate.x = pos.x;
	transform.translate.z = pos.y;
	player->SetPosition(ToXZ(transform.translate));

	timer_.Update();
	if (timer_.GetState() != TimerState::Finished) {
		return false;
	}

	if (mapCheck->IsFall(pos)) {
		player->Fall();
	}
	return true;
}
