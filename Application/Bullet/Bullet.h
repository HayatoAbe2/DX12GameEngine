#pragma once
#include "GameCommon.h"
#include "Weapon/WeaponStatus.h"
#include <memory>
#include <Timer/Timer.h>
#include "Effect/EffectManager.h"
#include <Character/Character.h>

class MapCheck;

class Bullet {
public:
	Bullet(const Vector2& direction,const BulletData& data, Character* from);
	~Bullet() = default;
	void Update(MapCheck* mapCheck, EffectManager* effectManager);
	void Draw(Camera* camera);
	void Hit();

	bool IsEnemyBullet() { return isEnemyBullet_; }
	const Circle& GetCollider() const  { return collider_; }
	float GetDamage() { return data_.damage; }
	float GetKnockback() { return data_.knockback; }
	bool IsDead() { return isDead_; }
	bool CanErase() { return canErase_; }
	Vector2 GetPrePos() { return prePos_; }
	bool CanHit() { return noHitTimer_.IsFinished(); }
	
public:
	void Move(MapCheck* mapCheck, EffectManager* effectManager);

	void OnHitWall(EffectManager* effectManager);
	void OnHitAnything(EffectManager* effectManager);

	// 弾ステータス
	BulletData data_;

	// 座標
	Circle collider_;

	// 前フレーム座標
	Vector2 prePos_{};
	// 速度
	Vector2 velocity_{};
	// 生存時間
	int lifeTime_ = 0;

	// 敵の弾フラグ
	bool isEnemyBullet_ = false;
	// 死亡フラグ(判定やめる)
	bool isDead_ = false;
	// 弾リストから削除可能
	bool canErase_ = false;
	
	// 無敵時間(ヒットした敵に一定時間当たらないリストに変更予定)
	Timer noHitTimer_;
	float invinsibleTime_ = 0.15f;

	// 射撃者
	Character* user_ = nullptr;

	// パーティクル
	std::unique_ptr<ParticleSystem> particle_;
	const int particleNum_ = 150;
	float particleRange_ = 0.7f;

	std::unique_ptr<ParticleSystem> hitParticle_;
	const int hitParticleNum_ = 10;
	float hitParticleRange_ = 1.0f;
	int hitParticleLifeTime = 20;

	std::unique_ptr<ParticleField> particleField_;
};

