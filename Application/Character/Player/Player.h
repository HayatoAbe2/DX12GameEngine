#pragma once
#include "GameCommon.h"
#include "Weapon/Weapon.h"
#include "Timer/Timer.h"
#include "Character/Character.h"
#include "Item/Passive/Passive.h"
#include "Wallet/Wallet.h"

class Camera;
class MapCheck;
class ItemManager;
class BulletManager;
class EnemyManager;
class MoneyUI;

class Player : public Character{
public:
	~Player();

	// 初期化
	void Initialize(std::unique_ptr<Model> playerModel, std::unique_ptr<Model> playerShadow, ItemManager* itemManager);
	// 更新
	void Update(MapCheck* mapCheck, Camera* camera, BulletManager* bulletManager);
	// 描画
	void Draw(Camera* camera);

	// 被弾
	void Hit(float damage, const Vector2& from);

	void OnHit(const Vector2& from, BulletManager* bulletManager);
	void OnDealDamage(const Vector2& pos, EnemyManager* enemyManager);

	void Move(MapCheck* mapCheck);
	void Trigger(BulletManager* bulletManager, Camera* camera);
	void Boost(MapCheck* mapCheck);
	void Fall();
	void Stun(MapCheck* mapCheck);
	Vector3 Raticle(Camera* camera);

	float GetRadius() const { return radius_; }
	float GetInteractRadius() const { return interactRadius_; }
	Weapon* GetWeapon() { return weapon_.get(); }
	Weapon* GetSubWeapon() { return subWeapon_.get(); }
	float GetHP() { return hp_; }
	float GetMaxHP() { return maxHp_; }
	Wallet& GetWallet() { return wallet_; }
	bool IsBoosting() { return isUsingBoost_; }
	bool IsInvincible() { return isUsingBoost_ || invincibleTimer_->IsActive(); }

	Transform GetTransform() const override { return transform_; }
	Vector3 GetPrePos() const override { return prePos_; }
	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetWeapon(std::unique_ptr<Weapon> weapon);
	void AddPassive(std::unique_ptr<Passive> passive);
	bool IsDead() { return hp_ <= 0; }

	void Stop() { boostTimer_->Reset(); }

private:
	// 照準
	std::unique_ptr<Model> direction_ = nullptr;

	// トランスフォーム
	Transform transform_;
	Transform weaponTransform_;

	// 半径
	float radius_ = 0.5f;

	// アイテム取得範囲
	float interactRadius_ = 1.5f;

	// 速度
	Vector3 velocity_ = { 0,0,0 };

	// 移動の速さ
	float moveSpeed_ = 10.0f;
	const float defaultMoveSpeed_ = 10.0f;

	// 攻撃の向き
	Vector2 attackDirection_ = {};

	// hp
	float hp_ = 100;
	float maxHp_ = 100;

	// スタン時間
	std::unique_ptr<Timer> stunTimer_ = nullptr;
	float stunTime_ = 0.6f;
	Vector2 knockbackVel_{};

	bool isFall_ = false;
	Vector3 landPos_{};
	std::unique_ptr<Timer> hitColorTimer_ = nullptr;
	std::unique_ptr<Timer> invincibleTimer_ = nullptr;
	float invincibleTimeOnHit_ = 1.0f;
	float invincibleTimeOnDodge_ = 0.25f;

	// モデル
	std::unique_ptr<Model> model_ = nullptr;
	std::unique_ptr<Model> shadowModel_ = nullptr;
	std::unique_ptr<Model> instancing_[2];
	Transform instancingTransforms[4]{};

	// ダッシュ
	bool canBoost_ = false;
	bool isUsingBoost_ = false;
	std::unique_ptr<Timer> boostTimer_;
	float maxBoostTime_ = 0.25f;
	float boostSpeed_ = 15.0f;
	Vector3 boostDir_{};
	int boostCoolTime_ = 10;

	// 射撃
	std::unique_ptr<Timer> shootCooldownTimer_;

	// 所持武器
	std::unique_ptr<Weapon> weapon_ = nullptr;
	std::unique_ptr<Weapon> subWeapon_ = nullptr;

	// 移動パーティクル
	std::unique_ptr<ParticleSystem> moveParticle_;
	const int moveParticleNum_ = 20;
	float moveParticleRange_ = 1.0f;
	int moveParticleEmitTimer_ = 0;
	int moveParticleEmitInterval_ = 20;

	float dirDisplayAlpha_ = 0.3f;

	// クールダウン表示
	std::unique_ptr<Sprite> shootCooldownSprite_;
	Vector2 scSize_ = {70,20};

	Vector3 prePos_{};

	std::vector<std::unique_ptr<Passive>> passives_;

	bool usePassive_[3];

	// 所持金
	Wallet wallet_;

	ItemManager* itemManager_ = nullptr;
};

