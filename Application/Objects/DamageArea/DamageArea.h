#pragma once
#include "GameCommon.h"
#include "Objects/Weapon/ShooterInfo/ShooterInfo.h"
#include "Timer/Timer.h"

enum class AreaPhase {
	Warning, // 警告表示 
	Active,
	Ended,

	Count
};

struct DamageAreaParam {
	Circle circle;
	Team team;

	float damage = 0;
	float delay = 0;
	float duration = 0;
	float hitInterval = 0.5f;
};

class DamageArea {
public:
	DamageArea(DamageAreaParam param, Team team);

	void Update();

	bool IsEnable() const { return phase_ == AreaPhase::Active; }
	Circle GetCollider() const { return param_.circle; }
	float GetDamage() const { return param_.damage; }
	Team GetTeam() const { return team_; };
	bool IsDead() const { return isDead_; }

private:
	DamageAreaParam param_;
	Team team_;

	// 現在フェーズ
	AreaPhase phase_ = AreaPhase::Warning;
	bool isDead_ = false;

	// 各フェーズタイマー
	Timer timer_;
};

