#include "DamageAreaManager.h"

void DamageAreaManager::Update() {
	for (auto& a : areas_) {
		a->Update();
	}
}

void DamageAreaManager::AddArea(DamageAreaParam param, Team team) {
	areas_.push_back(std::make_unique<DamageArea>(param, team));
}
