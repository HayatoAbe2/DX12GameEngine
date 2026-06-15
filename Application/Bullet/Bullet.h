#pragma once
#include "GameCommon.h"
#include "Weapon/WeaponStatus.h"
#include <memory>
#include <Timer/Timer.h>

class MapCheck;

class Bullet {
public:
	Bullet(std::unique_ptr<Model> model,const Vector3& direction,const WeaponData& data,bool isEnemyBullet);
	virtual ~Bullet() = default;
	virtual void Update(MapCheck* mapCheck) = 0;
	virtual void Draw(Camera* camera) = 0;
	virtual void Hit() = 0;

	bool IsEnemyBullet() { return isEnemyBullet_; }
	Transform GetTransform() { return model_->GetTransform(); }
	float GetDamage() { return data_.stats.damage; }
	float GetKnockback() { return data_.stats.knockback; }
	bool IsDead() { return isDead_; }
	bool CanErase() { return canErase_; }
	Vector3 GetPrePos() { return prePos_; }
	bool CanHit() { return noHitTimer_->IsFinished(); }
	
protected:
	// 前フレーム場所
	Vector3 prePos_{};

	// 速度
	Vector3 velocity_{};

	// モデル
	std::unique_ptr<Model> model_ = nullptr;

	// 弾ステータス
	WeaponData data_;

	// 生存時間
	int lifeTime_ = 0;

	// 敵の弾フラグ
	bool isEnemyBullet_ = false;

	// 死亡フラグ(判定やめる)
	bool isDead_ = false;

	// 弾リストから削除可能
	bool canErase_ = false;

	// 無敵時間
	std::unique_ptr<Timer> noHitTimer_;
	float invinsibleTime_ = 0.15f;
};

