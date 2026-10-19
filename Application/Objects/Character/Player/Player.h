#pragma once
#include "Objects/Weapon/Weapon.h"
#include "Objects/Character/Character.h"
#include "Objects/Item/Passive/Passive.h"
#include "Wallet/Wallet.h"

class Camera;
class MapCheck;
class ItemManager;
class BulletManager;
class EnemyManager;
class MoneyUI;

class Player : public Character {
public:
	~Player();

	// 初期化
	void Initialize(ItemManager* itemManager);
	// 更新
	void Update(MapCheck* mapCheck, Camera* camera, BulletManager* bulletManager);
	// 描画
	void Draw(Camera* camera);

	// 被弾
	void Hit(float damage, const Vector2& from);

	// 各パッシブアイテム発生イベント
	void OnDealDamage(const Vector2& pos);
	void OnEliminate(const Vector2& enemyPos);
	void OnHit(const Vector2& from);
	void OnDodge();

	void Move(MapCheck* mapCheck);
	void Trigger(BulletManager* bulletManager, Camera* camera);
	void Boost(MapCheck* mapCheck);
	void Fall();
	void Stun(MapCheck* mapCheck);

	float GetInteractRadius() const { return interactRadius_; }
	Weapon* GetWeapon() const { return weapon_.get(); }
	Weapon* GetSubWeapon() const { return subWeapon_.get(); }
	std::vector<Passive*> GetPassives() const {
		std::vector<Passive*> result;
		for (auto& p : passives_) {
			result.push_back(p.get());
		};
		return result;
	}
	Wallet& GetWallet() { return wallet_; }
	bool IsBoosting() { return isUsingBoost_; }

	void SetWeapon(std::unique_ptr<Weapon> weapon);
	void AddPassive(std::unique_ptr<Passive> passive);

	void Stop() { boostTimer_->Reset(); }

private:
	// 照準
	std::unique_ptr<Model> direction_ = nullptr;

	// 武器トランスフォーム
	Transform weaponTransform_;

	// アイテム取得範囲
	float interactRadius_ = 1.5f;

	// 速度
	Vector3 velocity_ = { 0,0,0 };

	// 移動の速さ
	float moveSpeed_ = 10.0f;
	const float defaultMoveSpeed_ = 10.0f;

	// 攻撃の向き
	Vector2 attackDirection_ = {};

	// スタン時間
	std::unique_ptr<Timer> stunTimer_ = nullptr;
	float stunTime_ = 0.6f;
	Vector2 knockbackVel_{};

	bool isFall_ = false;
	Vector3 landPos_{};
	std::unique_ptr<Timer> hitColorTimer_ = nullptr;
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
	Vector2 scSize_ = { 70,20 };

	std::vector<std::unique_ptr<Passive>> passives_;

	// 所持金
	Wallet wallet_;

	ItemManager* itemManager_ = nullptr;

	std::shared_ptr<Animation> walk_;
	std::shared_ptr<Animation> sneak_;
};

