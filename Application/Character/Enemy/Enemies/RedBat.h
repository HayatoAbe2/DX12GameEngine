#pragma once
#include "Character/Enemy/Enemy.h"
#include <memory>

class RedBat : public Enemy {
public:
	RedBat(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, const Vector3& pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) :
		Enemy(std::move(model), std::move(shadowModel), pos, status, move(rWeapons)) {
	};
	void Attack(BulletManager* bulletManager, const Vector2& dir) override;

private:
	const int maxCombo_ = 8;
	int comboCount_ = 0;
	const int maxComboInterval_ = 8;
	int comboInterval_ = 7;

	int weaponChangeTimer_ = 300;
	int weaponNum_ = 0;
};