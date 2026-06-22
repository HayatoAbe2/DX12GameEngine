#pragma once
#include "Weapon/Weapon.h"

class BulletManager;

class ChargeGun : public Weapon {
public:
	ChargeGun(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) :
		Weapon(data, std::move(model), std::move(shadowModel)) {
	}
	float Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) override;
	void Update() override;

private:
	float damageBonus_ = 0;
};