#pragma once
#include "Objects/Item/Passive/Passive.h"
#include "Objects/Item/World/WorldItem.h"

#include "Objects/Item/Passive/Counter/Counter.h"
#include "Objects/Item/Passive/Lightning/Lightning.h"
#include "Objects/Item/Passive/Reload/ReloadBoost.h"
#include "Objects/Item/Passive/Heal/Heal.h"
#include "Objects/Item/Passive/ExtraAttack/ExtraAttack.h"

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

