#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"
#include "Fade/Fade.h"

class FloorManager;
class Result {
public:
	Result(Fade* fade);

	void Update(FloorManager* floor);
	void Draw();

private:
	// リザルト
	Timer resultTimer_;
	float resultTime_ = 0;
	float resultArrowMove_ = 0;

	std::unique_ptr<Sprite> resultBG_ = nullptr;
	std::unique_ptr<Sprite> resultCursor_ = nullptr;

	// フェード
	Fade* fade_;
};

