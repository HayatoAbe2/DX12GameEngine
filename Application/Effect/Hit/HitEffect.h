#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

/// <summary>
/// ヒットエフェクト
/// </summary>
class HitEffect final {
public:
	void Initialize(const Vector3& pos, const Vector4& color);
	void Update();
	void Draw(Camera* camera);

	bool IsFinished() { return timer_.GetState() == TimerState::Finished; }
private:
	std::unique_ptr<Model> spark_;
	float size_ = 0;
	const float maxSize_ = 5.0f;

	Timer timer_;
	const float duration_ = 0.2f;
	const float peak_ = 0.4f;
};

