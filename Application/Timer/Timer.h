#pragma once

// 汎用タイマークラス
class Timer {
public:
    void Start(float duration);
    void Update();

    void AddTime(float additionalTime);

    bool IsFinished() const {
        return !isActive_;
    }

    bool IsActive() const {
        return isActive_;
    }

    float GetRemaining() const {
        return time_;
    }

private:
    float time_ = 0.0f;
    bool isActive_ = false;
};