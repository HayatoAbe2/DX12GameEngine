#pragma once
#include "GameCommon.h"
#include "WeaponStatus.h"
#include "Character/Character.h"
#include "Timer/Timer.h"

class BulletManager;

class Weapon {
public:
	Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel);
	virtual ~Weapon() = default;
	virtual float Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) = 0;
	virtual void Update();

	Model* GetWeaponModel() { return model_.get(); }
	Model* GetWeaponShadowModel() { return shadowModel_.get(); }
	// ステータス(強化分)
	WeaponModifier& GetModifier() { return modifier_; }
	// データ(基礎数値)
	const WeaponData& GetData() const { return data_; }
	
	float GetCharge() { return charge_; }
protected:
	void SetChargeStartTimer();

	WeaponData data_;
	WeaponModifier modifier_{};
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> shadowModel_;

	std::unique_ptr<Timer> chargeStartTimer_;
	float charge_;
};

