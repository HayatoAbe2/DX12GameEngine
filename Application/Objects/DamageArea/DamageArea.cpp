#include "DamageArea.h"

DamageArea::DamageArea(DamageAreaParam param, Team team) {
	param_ = param;
	team_ = team;

	timer_.Start(param_.delay);
}

void DamageArea::Update() {
	// タイマー処理
	timer_.Update();

	switch (phase_) {
	case AreaPhase::Warning:
		if (timer_.IsFinished()) {
			phase_ = AreaPhase::Active;
			timer_.Start(param_.duration);
		}
		break;

	case AreaPhase::Active:
		if (timer_.IsFinished()) {
			phase_ = AreaPhase::Ended;
			timer_.Start(0);
		}
		break;

	case AreaPhase::Ended:
		if (timer_.IsFinished()) {
			isDead_ = true;
		}
		break;
	}
}
