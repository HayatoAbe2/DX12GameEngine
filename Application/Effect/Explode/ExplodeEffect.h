#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

/// <summary>
/// 爆発エフェクト
/// </summary>
class ExplodeEffect final {
public:
	ExplodeEffect(const Vector3 pos, const Vector4 color, const float maxSize);
	void Update();
	void Draw(Camera* camera);

	bool IsFinished() {
		return timer_.GetState() == TimerState::Finished;
	}
	std::unique_ptr<Model> dome_;
	float size_ = 0;
	float maxSize_ = 5.0f;

	Timer timer_;
	const float duration_ = 0.5f;
	float alphaStart_ = 0.5f;
};
