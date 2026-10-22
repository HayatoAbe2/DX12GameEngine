#pragma once
#include "Objects/Character/Enemy/Enemy.h"
#include <memory>
class HeavyKnight : public Enemy {
public:
	HeavyKnight(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, const Vector3& pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) :
		Enemy(std::move(model), std::move(shadowModel), pos, status, move(rWeapons)) {}
	void Attack(BulletManager* bulletManager, const Vector2& dir) override;

private:
	const int maxCombo_ = 4;
	int comboCount_ = 0;
	const int maxComboInterval_ = 15;
	int comboInterval_ = 15;

	int weaponChangeTimer_ = 60;
	int weaponNum_ = 0;
};

