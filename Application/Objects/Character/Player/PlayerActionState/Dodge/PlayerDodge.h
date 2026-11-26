#pragma once
#include "Objects/Character/Player/PlayerActionState/PlayerActionState.h"
#include "Timer/Timer.h"
#include "GameCommon.h"

/// <summary>
/// 回避行動
/// </summary>
class PlayerDodge final : public PlayerActionState {
public:
	PlayerDodge();
	bool Update(Player* player, MapCheck* mapCheck) override;
	PlayerState GetCurrentState() const override { return PlayerState::Dodge; }

private:
	Vector3 direction_{};
	float speed_ = 15.0f;
	const float duration_ = 0.25f;
	Timer timer_;
};
