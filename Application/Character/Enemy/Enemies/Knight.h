#pragma once
#include "Character/Enemy/Enemy.h"
#include <memory>
class Knight : public Enemy {
public:
	Knight(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, Vector3 pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) :
		Enemy(std::move(model), std::move(shadowModel), pos, status, move(rWeapons)) {
	}
	void Attack(BulletManager* bulletManager, const Vector3& dir, Camera* camera) override;

private:
	const int maxCombo_ = 4;
	int comboCount_ = 0;
	const int maxComboInterval_ = 10;
	int comboInterval_ = 10;
};

