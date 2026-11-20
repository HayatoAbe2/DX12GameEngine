#include "Timer.h"
#include "GameCommon.h"

void Timer::Start(float duration) {
	time_ = duration;
    startTime_ = duration;
    state_ = TimerState::Running;
}

void Timer::Update() {
    if (state_ != TimerState::Running) return;
    auto& ctx = GameContext::GetInstance();

    time_ -= ctx.GetDeltatime();

    if (time_ <= 0.0f) {
        time_ = 0.0f;
        state_ = TimerState::Finished;
    }
}

void Timer::AddTime(float additionalTime) {
    if (additionalTime <= 0.0f) {
        return;
    }

    time_ += additionalTime;
}