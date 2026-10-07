#pragma once
#include "GameCommon.h"
#include "Weapon/Weapon.h"
#include "Bullet/BulletManager.h"
#include "Character/Enemy/EnemyManager/EnemyManager.h"
#include "Item/Item.h"
#include "Timer/Timer.h"

struct PassiveContext {
	EnemyManager* enemyManager;
	BulletManager* bulletManager;
};

class Passive : public Item {
public:
	~Passive() = default;

	Passive(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx);

	Sprite* GetSprite() { return sprite_.get(); }
	Sprite* GetExplain() { return explain_.get(); }
	void Update();
	void Draw();

	// 常時
	virtual void OnUpdate(Weapon* weapon, Weapon* subWeapon) {}
	// 与ダメージ時
	virtual void OnDealDamage(const Vector2& pos) {}
	// 敵撃破時
	virtual void OnEliminate(const Vector2& enemyPos, Character* self) {}
	// 被弾時
	virtual void OnHit(const Vector2& pos, Character* self) {}
	// 回避時
	virtual void OnDodge(Character* self) {}

private:
	std::unique_ptr<Sprite> sprite_;
	std::unique_ptr<Sprite> explain_;

protected:
	PassiveContext passiveCtx_;

	Timer coolDowntimer_;
};

