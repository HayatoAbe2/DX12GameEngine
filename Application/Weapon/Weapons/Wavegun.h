#pragma once
#include "Weapon/Weapon.h"

class BulletManager;

class Wavegun : public Weapon {
public:
	Wavegun(const WeaponStatus& status, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel);
	int Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, bool isEnemyBullet);
	void Update() override;

private:
};

