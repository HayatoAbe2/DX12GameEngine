#include "Timer.h"

void Timer::Start(float duration) {
	time_ = duration;
	isActive_ = true;
}

void Timer::Update() {
    if (!isActive_) return;

    time_ -= 1.0f / 60.0f;

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

