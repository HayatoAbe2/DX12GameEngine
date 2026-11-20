#pragma once

enum class TimerState {
    Idle,       // 開始前
    Running,    // 計測中
    Finished    // 終了
};

/// <summary>
/// 汎用タイマークラス
/// </summary>
class Timer {
public:
    void Start(float duration);
    void Update();

    // 時間追加
    void AddTime(float additionalTime);

    // リセット
    void Reset() { time_ = 0; state_ = TimerState::Idle; }

    TimerState GetState() const {
        return state_;
    }
    bool IsActive() const {
        return state_ == TimerState::Running;
    }

    float GetRemaining() const {
        return time_;
    }
    float GetStartTime() const {
        return startTime_;
    }

private:
    float time_ = 0.0f;
    float startTime_ = 0.0f;
    TimerState state_ = TimerState::Idle;
};