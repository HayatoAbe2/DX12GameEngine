#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

class ExplodeEffect {
public:
	void Initialize(const Vector3& pos, const Vector4& color, const float maxSize);
	void Update();
	void Draw(Camera* camera);

	bool IsFinished() { return timer_->IsFinished(); }
private:
	std::unique_ptr<Model> dome_;
	float size_ = 0;
	float maxSize_ = 5.0f;

	std::unique_ptr<Timer> timer_;
	const float duration_ = 0.5f;
	float alphaStart_ = 0.5f;
};
