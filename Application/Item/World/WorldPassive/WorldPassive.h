#pragma once
#include "Item/Passive/Passive.h"
#include "Item/World/WorldItem.h"

#include "Item/Passive/Counter/Counter.h"
#include "Item/Passive/Lightning/Lightning.h"
#include "Item/Passive/Reload/ReloadBoost.h"

// 落ちている状態
class WorldPassive : public WorldItem {
public:
	// コンストラクタで所有権移転
	WorldPassive(std::unique_ptr<Passive> passive, Vector3 pos, Rarity rarity, bool isForSale = false);

	std::unique_ptr<Passive> GetPassiveItem();

	void Update() override;
	void Draw() override;

	void OnPickup(Player* player) override;
private:
	std::unique_ptr<Passive> passive_ = nullptr;
};

