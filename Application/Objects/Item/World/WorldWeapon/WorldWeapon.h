#pragma once
#include "Objects/Weapon/Weapon.h"
#include "Objects/Item/World/WorldItem.h"

/// <summary>
/// 落ちている武器
/// </summary>
class WorldWeapon : public WorldItem {
public:
	// コンストラクタで所有権移転
	WorldWeapon(std::unique_ptr<Weapon> weapon, Vector3 pos, Rarity rarity, bool isForSale = false);

	std::unique_ptr<Weapon> GetWeapon();
	const WeaponData& GetWeaponData();

	void Update() override;
	void Draw() override;
	void DrawSummary();

	void OnPickup(Player* player) override;
private:
	std::unique_ptr<Weapon> weapon_ = nullptr;
};

