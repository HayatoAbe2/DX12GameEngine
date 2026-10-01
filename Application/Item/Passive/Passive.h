#pragma once
#include "GameCommon.h"
#include "Weapon/Weapon.h"
#include "Bullet/BulletManager.h"
#include "Character/Enemy/EnemyManager.h"
#include "Item/Item.h"

struct PassiveContext {
	EnemyManager* enemyManager;
	BulletManager* bulletManager;
};

class Passive : public Item {
public:
	~Passive() = default;

	Passive(std::unique_ptr<Sprite> sprite, PassiveContext ctx);

	Sprite* GetSprite() { return sprite_.get(); }
	void Draw();

	virtual void OnUpdate(Weapon* weapon, Weapon* subWeapon) {}
	virtual void OnDealDamage(const Vector2& pos){}
	virtual void OnHit(const Vector2& pos, Character* self){}

private:
	std::unique_ptr<Sprite> sprite_;

protected:
	PassiveContext passiveCtx_;
};

