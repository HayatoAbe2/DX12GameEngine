#pragma once
#include "GameCommon.h"
#include "WeaponStatus.h"
#include "Character/Character.h"
#include "Timer/Timer.h"
#include "Item/Item.h"

class BulletManager;

class Weapon : public Item {
public:
	Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel);
	~Weapon() = default;

	float Trigger(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from);
	void Update(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from);

	Model* GetModel() { return model_.get(); }
	Model* GetShadowModel() { return shadowModel_.get(); }

	// ステータス(強化分)
	std::array<ModifierType, size_t(ModifierStats::count)>& GetModifier() { return data_.modifiers; }

	// データ(基礎数値)
	const WeaponData& GetData() const { return data_; }
	
	float GetCharge() { return charge_; }
	bool CanChange() { return canChange_; }
protected:
	float Fire(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from);
	void SetChargeStartTimer();

	WeaponData data_;
	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> shadowModel_;

	Timer reloadStartTimer_;
	float charge_ = 0;

	bool canChange_ = true;
};

