#pragma once
#include "Weapon/Weapon.h"
#include "Timer/Timer.h"

class BulletManager;

class Burstgun : public Weapon {
public:
	Burstgun(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) :
		Weapon(data, std::move(model), std::move(shadowModel)) {
		burstTimer_[0] = std::make_unique<Timer>();
		burstTimer_[1] = std::make_unique<Timer>();
	}
	float Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) override;
	void Update() override;

private:
	BulletManager* bulletManager_ = nullptr;
	Character* from_ = nullptr;
	std::unique_ptr<Timer> burstTimer_[2];
	bool burst_[2]{};
};