#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

class Chest {
public:
	Chest(Vector2 position);
	void Update();
	void Draw();

	enum class ChestPhase {
		Closed,
		PlayingAnimation,
		Opened,
		Die
	};

	// アイテム排出
	bool CheckDrop();

	bool IsDead() { return isDead_; }
	Vector2 GetPosition() { return position_; }
private:
	// モデル
	std::unique_ptr<Model> model_;
	std::shared_ptr<Animation> anim_;
	const int kOpenAnimNumber_ = 1;

	// 状態
	ChestPhase phase_ = ChestPhase::Closed;
	bool isDead_ = false;

	// 位置
	Vector2 position_{};
	const float kSpawnHeight = 5.0f;
	float height_ = kSpawnHeight;

	// タイマー
	Timer appierTimer_;
	Timer dyingTimer_;
	const float kAppierTime = 1.0f;
	const float kDyingTime = 1.5f;
};

