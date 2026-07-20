#pragma once
#include "Weapon/Weapon.h"

class BulletManager;

class Wavegun : public Weapon {
public:
	Wavegun(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) :
		Weapon(data, std::move(model), std::move(shadowModel)) {
	}
	float Trigger(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from);
	void Update() override;

private:
};

