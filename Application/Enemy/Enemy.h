#pragma once
#include "Weapon/Weapon.h"
#include "EnemyStatus.h"
#include "Bullet/BulletManager.h"
#include "Timer/Timer.h"
#include "GameCommon.h"
#include "Enemy/EnemyState/EnemyState.h"

class MapCheck;
class Player;

class Enemy {
public:
	virtual ~Enemy() = default;
	Enemy(std::unique_ptr<Model> model, std::unique_ptr<Model> shadow, Vector3 pos, EnemyStatus status,std::vector<std::unique_ptr<Weapon>> rWeapons);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, Camera* camera);

	// Update内関数
	void Stun(MapCheck* mapCheck);
	void Fall();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="context">コンテキスト</param>
	/// <param name="camera">カメラ</param>
	void Draw(Camera* camera);

	// 被ダメージ時
	void Hit(float damage,Vector3 from, const float knockback);

	// 敵ごとの関数
	virtual void Attack(BulletManager* bulletManager, const Vector3& dir, Camera* camera) = 0;

	Transform GetTransform() const { return model_->GetTransform(); }
	void SetTransform(const Transform& transform) const { model_->SetTransform(transform); }
	Timer* GetInvinsibleTimer() const { return invincibleTimer_.get(); }
	float GetRadius() const { return status_.radius; }
	bool IsDead() { return isDead_; }

	EnemyStatus GetStatus() { return status_; }

	void SetState(std::unique_ptr<EnemyState> state) { currentState_ = std::move(state); }
	bool IsAttacking() { return isAttacking_; }
	Weapon* GetCurrentWeapon() { return currentWeapon_; }
	Timer* GetAttackTimer() { return attackCoolTimer_.get(); }
	void Slow() { slowTimer_->Start(5.0f); }
protected:

	// 敵別ステータス
	EnemyStatus status_;

	// 移動
	Vector3 velocity_{};

	// モデル
	std::unique_ptr<Model> model_ = nullptr;
	std::unique_ptr<Model> shadowModel_ = nullptr;

	// 武器
	std::vector<std::unique_ptr<Weapon>> weapons_; // 複数ある場合
	Weapon* currentWeapon_;

	// 射撃クールダウン
	std::unique_ptr<Timer> attackCoolTimer_;

	// プレイヤー
	Player* target_ = nullptr;

	// 発見範囲
	float searchRadius_ = 8.0f;

	// 死亡フラグ
	bool isDead_ = false;

	// スタン時間
	std::unique_ptr<Timer> stunTimer_ = nullptr;
	int hitColorTime_ = 0;

	// 落下
	bool isFall_ = false;

	// プレイヤーに近づく最小距離
	float minDistance_ = 10.0f;

	// 無敵時間
	std::unique_ptr<Timer> invincibleTimer_;
	float invinsibleTime_ = 0.15f;

	// 行動状態
	std::unique_ptr<EnemyState> currentState_ = nullptr;

	// 攻撃中
	bool isAttacking_ = false;

	std::unique_ptr<Timer> slowTimer_;
};