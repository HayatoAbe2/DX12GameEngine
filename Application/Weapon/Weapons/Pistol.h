#pragma once
#include "Weapon/Weapon.h"

class BulletManager;

class Pistol : public Weapon {
public:
	Pistol(const WeaponStatus& status, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel);
	int Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, bool isEnemyBullet) override;
	void Update() override;

private:
};