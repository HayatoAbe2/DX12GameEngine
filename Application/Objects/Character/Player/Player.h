#pragma once
#include "Objects/Weapon/Weapon.h"
#include "Objects/Character/Character.h"
#include "Objects/Item/Passive/Passive.h"
#include "Wallet/Wallet.h"
#include "Objects/Character/Player/PlayerActionState/PlayerActionState.h"

class Camera;
class MapCheck;
class ItemManager;
class BulletManager;
class EnemyManager;
class MoneyUI;

/// <summary>
/// プレイヤー
/// </summary>
class Player : public Character {
public:
	~Player();

	// 初期化
	void Initialize(ItemManager* itemManager);
	// 更新
	void Update(MapCheck* mapCheck, Camera* camera, BulletManager* bulletManager);
	// 描画
	void Draw(Camera* camera);
	// アウトライン用マスクの描画(OutlineMaskパス中)
	void DrawOutline();

	// 被弾
	void Hit(float damage, const Vector2& from);

	// 各パッシブアイテム発動条件のイベント
	void OnDealDamage(const Vector2& pos);
	void OnEliminate(const Vector2& enemyPos);
	void OnHit(const Vector2& from);
	void OnDodge();

	void Move(MapCheck* mapCheck); 
	void Trigger(BulletManager* bulletManager, Camera* camera);

	// 状態
	void Stun(MapCheck* mapCheck);
	void OnStunEnd() override;
	void OnFallEnd() override;

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
	bool IsDodging() { return actionState_->GetCurrentState() == PlayerState::Dodge; }

	void SetWeapon(std::unique_ptr<Weapon> weapon);
	void AddPassive(std::unique_ptr<Passive> passive);

	// 現在の行動状態
	PlayerState GetCurrentState();

	void Stop();

private:

	// 照準
	std::unique_ptr<Model> direction_ = nullptr;

	// 武器トランスフォーム
	Transform weaponTransform_;

	// アイテム取得範囲
	float interactRadius_ = 1.5f;

	// 移動の速さ
	float moveSpeed_ = 10.0f;
	const float defaultMoveSpeed_ = 10.0f;

	// 攻撃の向き
	Vector2 attackDirection_ = {};

	// スタン時間
	float stunTime_ = 0.6f;
	Vector2 knockbackVel_{};

	MapCheck* mapCheck_ = nullptr;
	Vector3 landPos_{};
	Timer hitColorTimer_;
	float invincibleTimeOnHit_ = 1.0f;
	float invincibleTimeOnDodge_ = 0.25f;

	// モデル
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> shadowModel_;
	std::unique_ptr<Model> instancing_[2];
	Transform instancingTransforms[4]{};

	// アクション状態
	std::unique_ptr<PlayerActionState> actionState_;

	// ダッシュ
	bool canBoost_ = false;
	int boostCoolTime_ = 10;

	// 射撃
	Timer shootCooldownTimer_;

	// 所持武器
	std::unique_ptr<Weapon> weapon_;
	std::unique_ptr<Weapon> subWeapon_;

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

	// アニメーション
	std::shared_ptr<Animation> walk_;
	std::shared_ptr<Animation> sneak_;

	// ライト
	int lightNumber_ = -1;
	const float directionFactor = 0.5f;
	const float kLightHeight = 1.0f;
};

