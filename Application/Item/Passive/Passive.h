#pragma once
#include "GameCommon.h"
#include "Weapon/Weapon.h"
#include "Bullet/BulletManager.h"
#include "Character/Enemy/EnemyManager.h"
#include "Item/Item.h"

class Passive : public Item {
public:
	Passive();
	~Passive() = default;

	Passive(Sprite* sprite);

	void Draw();

	virtual void OnUpdate(Weapon* weapon, Weapon* subWeapon) {}
	virtual void OnDealDamage(const Vector2& pos, EnemyManager* enemyManager){}
	virtual void OnHit(const Vector2& pos, BulletManager* bulletManager, Character* self){}

private:
	Sprite* sprite_;
};

