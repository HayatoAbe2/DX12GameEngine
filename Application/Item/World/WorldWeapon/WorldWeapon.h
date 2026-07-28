#pragma once
#include "Weapon/Weapon.h"
#include "Item/World/WorldItem.h"

// 落ちている状態
class WorldWeapon : public WorldItem {
public:
	// コンストラクタで所有権移転
	WorldWeapon(std::unique_ptr<Weapon> weapon, Vector3 pos, Rarity rarity, bool isForSale = false);

	std::unique_ptr<Weapon> GetWeapon();

	void Update() override;
	void Draw() override;

	void OnPickup(Player* player) override;
private:
	std::unique_ptr<Weapon> weapon_ = nullptr;
};

