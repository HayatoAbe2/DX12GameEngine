#pragma once
#include "Weapon/Weapon.h"
#include "Rarity.h"
#include <memory>

class Camera;

class Item {
public:
	~Item();
	Item(std::unique_ptr<Weapon> weapon, Vector3 pos,Rarity rarity);
	void Draw(Camera* camera);

	Transform GetTransform() { if (weapon_) { return weapon_->GetWeaponModel()->GetTransform(); } return{}; }
	std::unique_ptr<Weapon> GetWeapon() { return std::move(weapon_); }
	bool IsDead() { return isDead_; }
	void Erase();
private:
	// モデル
	std::unique_ptr<Weapon> weapon_ = nullptr;

	// ライトのインデックス
	int lightIndex_ = 0;

	bool isDead_ = false;

	// レア度
	Rarity rarity_{};
};

