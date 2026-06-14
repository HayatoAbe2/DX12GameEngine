#pragma once
#include <chrono>

class FixFPS {
public:
	void Initialize();
	void Update();

	float GetDeltatime() { return deltatime_; }
private:
	std::chrono::steady_clock::time_point reference_;
	float deltatime_ = 0;
};

