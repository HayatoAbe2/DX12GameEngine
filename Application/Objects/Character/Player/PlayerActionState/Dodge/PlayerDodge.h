#pragma once
#include "Objects/Character/Player/PlayerActionState/PlayerActionState.h"
#include "Timer/Timer.h"
#include "GameCommon.h"

/// <summary>
/// 回避行動
/// </summary>
class PlayerDodge final : public PlayerActionState {
public:
	PlayerDodge(const Vector3& direction, float speed, float duration);
	bool Update(Player* player, MapCheck* mapCheck) override;

private:
	Vector3 direction_{};
	float speed_ = 0.0f;
	Timer timer_;
};
