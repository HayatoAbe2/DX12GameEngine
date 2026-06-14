#include "Timer.h"
#include "GameCommon.h"

void Timer::Start(float duration) {
	time_ = duration;
	isActive_ = true;
}

void Timer::Update() {
    if (!isActive_) return;
    auto& ctx = GameContext::GetInstance();

    time_ -= ctx.GetDeltatime();

    if (time_ <= 0.0f) {
        time_ = 0.0f;
        isActive_ = false;
    }
}

void Timer::AddTime(float additionalTime) {
    if (additionalTime <= 0.0f) {
        return;
    }

    time_ += additionalTime;
}

