#pragma once
#include "GameCommon.h"
#include "Weapon/Weapon.h"
#include "Bullet/BulletManager.h"
#include "Character/Enemy/EnemyManager.h"
#include "Item/Item.h"

class Passive : public Item {
public:
	~Passive() = default;

	Passive(std::unique_ptr<Sprite> sprite);

	Sprite* GetSprite() { return sprite_.get(); }
	void Draw();

	virtual void OnUpdate(Weapon* weapon, Weapon* subWeapon) {}
	virtual void OnDealDamage(const Vector2& pos, EnemyManager* enemyManager){}
	virtual void OnHit(const Vector2& pos, BulletManager* bulletManager, Character* self){}

private:
	std::unique_ptr<Sprite> sprite_;
};

