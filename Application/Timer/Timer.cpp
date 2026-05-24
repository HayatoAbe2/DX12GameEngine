#include "Timer.h"

void Timer::Start(float duration) {
	time_ = duration;
	isActive_ = true;
}

void Timer::Update(float deltaTime) {
    if (!isActive_) return;

    time_ -= deltaTime;

    if (time_ <= 0.0f) {
        time_ = 0.0f;
        isActive_ = false;
    }
}

