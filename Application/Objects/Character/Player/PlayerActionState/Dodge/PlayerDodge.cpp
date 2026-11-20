#include "PlayerDodge.h"
#include "Objects/Character/Player/Player.h"
#include "Map/MapCheck.h"
#include <cmath>
#include <numbers>

PlayerDodge::PlayerDodge(const Vector3& direction, float speed, float duration)
	: direction_(direction), speed_(speed) {
	timer_.Start(duration);
}

bool PlayerDodge::Update(Player* player, MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	player->transform_.rotate.y = -std::atan2(direction_.z, direction_.x) + std::numbers::pi_v<float> / 2.0f;
	Vector3 velocity = direction_ * speed_ * ctx.GetDeltatime();
	Vector2 pos = { player->transform_.translate.x, player->transform_.translate.z };
	for (int i = 0; i < 4; ++i) {
		pos.x += velocity.x / 4.0f;
		mapCheck->ResolveCollisionX(pos, player->radius_, true);
		pos.y += velocity.z / 4.0f;
		mapCheck->ResolveCollisionY(pos, player->radius_, true);
	}
	player->transform_.translate.x = pos.x;
	player->transform_.translate.z = pos.y;

	timer_.Update();
	if (timer_.GetState() != TimerState::Finished) {
		return false;
	}

	player->isUsingBoost_ = false;
	if (mapCheck->IsFall(pos)) {
		player->isFall_ = true;
		player->Character::Fall();
		ctx.Audio().SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
	}
	return true;
}
