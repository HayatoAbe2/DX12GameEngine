#pragma once
#include "Enemy/Enemy.h"
#include <memory>

class Bat : public Enemy {
public:
	Bat(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, Vector3 pos, EnemyStatus status, std::unique_ptr<Weapon> rWeapon) :
		Enemy(std::move(model), std::move(shadowModel), pos, status, move(rWeapon)) {
	};
	void Attack(Weapon* weapon, BulletManager* bulletManager, Camera* camera) override;

private:
};