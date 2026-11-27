#pragma once
#include "Objects/Weapon/Weapon.h"
#include "EnemyStatus.h"
#include "EnemyData.h"
#include "Objects/Bullet/BulletManager.h"
#include "Timer/Timer.h"
#include "GameCommon.h"
#include "Objects/Character/Enemy/EnemyActionState/EnemyActionState.h"
#include "Objects/Character/Character.h"

class MapCheck;
class Player;

/// <summary>
/// 敵キャラ
/// </summary>
class Enemy final : public Character {
public:
	virtual ~Enemy() = default;
	Enemy(std::unique_ptr<Model> model, std::unique_ptr<Model> shadow, Vector3 pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons, WeaponSwitch weaponSwitch = {});

	/// <summary>
	/// 更新
	/// </summary>
	void Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager);

	// Update内関数
	void Stun(MapCheck* mapCheck);
	void OnStunEnd() override;
	void OnFallEnd() override;

	// 描画
	void Draw();
	// アウトライン用マスクの描画(OutlineMaskパス中に、Drawの後で呼ぶ)
	void DrawOutline();

	// 被ダメージ時
	void Hit(float damage, const Vector2& from, const float knockback);

	// 敵ごとの関数
	void Attack(BulletManager* bulletManager);

	bool IsDead() { return isDead_; }

	EnemyStatus GetStatus() const { return status_; }

	void SetAttackDir(Vector2 dir) { attackDir_ = dir; }
	void SetState(std::unique_ptr<EnemyActionState> state) { nextState_ = std::move(state); }
	bool IsAttacking() { return isAttacking_; }
	Weapon* GetCurrentWeapon() { return currentWeapon_; }
	Timer& GetAttackTimer() { return attackCoolTimer_; }
	void Slow() { slowTimer_.Start(5.0f); }

private:

	// 敵別ステータス
	EnemyStatus status_;

	// 移動
	Vector2 knockbackVelocity_{};

	// モデル
	std::unique_ptr<Model> model_ = nullptr;
	std::unique_ptr<Model> shadowModel_ = nullptr;
	// アニメーション
	std::array<std::shared_ptr<Animation>, static_cast<int>(EnemyAnimations::Count)> animations;

	// 武器
	std::vector<std::unique_ptr<Weapon>> weapons_; // 複数ある場合
	Weapon* currentWeapon_ = nullptr;

	// 複数の武器の切り替え(一定回数攻撃したら次の武器へ)
	WeaponSwitch weaponSwitch_;
	int weaponIndex_ = 0;
	int remainingAttacks_ = 1;   // 今の武器で、あと何回攻撃するか
	int RollAttackCount() const;

	// 射撃クールダウン
	Timer attackCoolTimer_;

	// 攻撃方向
	Vector2 attackDir_;

	// プレイヤー
	Player* target_ = nullptr;

	// 発見範囲
	float searchRadius_ = 8.0f;

	// スタン時間
	Timer hitColorTimer_;

	// map判定
	MapCheck* mapCheck_ = nullptr;

	// プレイヤーに近づく最小距離
	float minDistance_ = 10.0f;

	// 行動状態
	std::unique_ptr<EnemyActionState> currentState_ = nullptr;
	std::unique_ptr<EnemyActionState> nextState_ = nullptr;

	// 攻撃中
	bool isAttacking_ = false;

	Timer slowTimer_;

	// 出現しおわるまでの時間
	Timer appierTimer_;
	const float kAppierTime = 0.7f;

};
