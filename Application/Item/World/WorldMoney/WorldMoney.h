#pragma once
#include "Item/World/WorldItem.h"

// 落ちている状態
class WorldMoney : public WorldItem {
public:
	// 出現
	WorldMoney(Vector3 pos, int amount);

	void Update() override;
	void Draw() override;

	void OnPickup(Player* player) override;
private:
	int money_ = 0;

	std::unique_ptr<Model> model_ = nullptr;
};


