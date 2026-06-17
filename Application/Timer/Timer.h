#pragma once

// 汎用タイマークラス
class Timer {
public:
    void Start(float duration);
    void Update();

    // 時間追加
    void AddTime(float additionalTime);

    // リセット
    void Reset() { time_ = 0; isActive_ = false; }

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