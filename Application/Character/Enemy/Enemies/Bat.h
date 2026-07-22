#pragma once
#include "Character/Enemy/Enemy.h"
#include <memory>

class Bat : public Enemy {
public:
	Bat(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, const Vector3& pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) :
		Enemy(std::move(model), std::move(shadowModel), pos, status, move(rWeapons)) {
	};
	void Attack(BulletManager* bulletManager, const Vector2& dir) override;

private:
};