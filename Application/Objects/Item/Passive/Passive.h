#pragma once
#include "GameCommon.h"
#include "Objects/Weapon/Weapon.h"
#include "Objects/Bullet/BulletManager.h"
#include "Objects/Character/Enemy/EnemyManager/EnemyManager.h"
#include "Objects/Item/Item.h"
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
	Sprite* GetDescription() { return description_.get(); }
	Sprite* GetBackground() { return background_.get(); }
	void Draw();
	void DrawDescription(); // 説明の表示

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
	std::unique_ptr<Sprite> description_; // アイテム説明
	std::unique_ptr<Sprite> background_;
	const Vector2 kSpriteSize = {80.0f, 80.0f};
	const float kDescriptionScale = 0.2f;
	const float kBackgroundScale = 1.2f;
protected:
	PassiveContext passiveCtx_;

	Timer coolDowntimer_;
};

