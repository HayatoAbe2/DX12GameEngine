#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

const float kMaxFadeinTimer_ = 1.0f;
const float kMaxFadeoutTimer_ = 1.0f;

enum class FadePhase {
	None,
	FadeOut,
	Faded,
	FadeIn
};

class Fade {
private:
	FadePhase fadePhase_ = FadePhase::None;
public:
	Fade();
	void Update();
	void Draw();

	// 開始
	void StartFadeIn(float seconds = kMaxFadeinTimer_);
	void StartFadeOut(float seconds = kMaxFadeoutTimer_);

	bool IsActive() { return timer_.IsActive(); }
	FadePhase GetPhase() { return fadePhase_; }

private:
	Timer timer_;
	float maxTime_ = kMaxFadeinTimer_;

	bool isFinished_ = false;

	std::unique_ptr<Sprite> sprite_;

};

