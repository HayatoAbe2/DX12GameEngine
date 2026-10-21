#pragma once
#include "GameCommon.h"
#include "Objects/Weapon/ShooterInfo/ShooterInfo.h"
#include "Timer/Timer.h"

class Character {
public:
	// トランスフォーム
	virtual Transform GetTransform() { return transform_; }
	virtual void SetTransform(const Transform& transform) { transform_ = transform; }
	virtual void SetPosition(const Vector2 position) { transform_.translate = { position.x, transform_.translate.y, position.y }; }

	// 前フレーム位置
	virtual Vector3 GetPrePos() { return prePos_; }

	// 半径
	float GetRadius() const { return radius_; }

	// 重さ
	float GetWeight() const { return weight_; }

	// チーム
	Team GetTeam() const { return team_; }

	// HP
	float GetHP() { return hp_; }
	float GetMaxHP() { return maxHp_; }

	// 死亡状態
	bool IsDead() { return isDead_; }

	// 飛行
	bool CanFly() { return canFly_; }

	// 無敵
	bool IsInvincible() { return invincibleTimer_.IsActive(); }

	// 回復
	void Heal(int addHP, int addMaxHP = 0) {
		hp_ += addHP;
		maxHp_ += addMaxHP;
		hp_ = (std::min)(hp_, maxHp_);
	}
protected:
	// 被弾
	virtual void Hit(float damage, const Vector2& from);

	// 行動不能
	virtual void Stun();

	// 落下
	virtual void Fall();

	// トランスフォーム
	Transform transform_;
	Vector3 prePos_{};

	// 半径
	float radius_ = 0.5f;

	// 重さ
	float weight_ = 10;

	// チーム
	Team team_;

	// hp
	float hp_ = 100;
	float maxHp_ = 100;

	// 死亡
	bool isDead_ = false;

	// 飛行する
	bool canFly_ = false;

	// 無敵タイマー
	Timer invincibleTimer_;
};