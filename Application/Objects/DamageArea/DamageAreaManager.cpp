#include "DamageAreaManager.h"

void DamageAreaManager::Update() {
	// 更新
	for (auto& a : areas_) {
		a->Update();
	}

	// 削除
	std::erase_if(areas_, [](const std::unique_ptr<DamageArea>& a) {
		return a->IsDead();
		}
	);
}

void DamageAreaManager::AddArea(DamageAreaParam param, Team team) {
	areas_.push_back(std::make_unique<DamageArea>(param, team));
}
