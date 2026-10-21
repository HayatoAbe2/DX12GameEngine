#pragma once
#include "DamageArea.h"
class DamageAreaManager {
public:
	void Update();

	void AddArea(DamageAreaParam param, Team team);
	std::vector<DamageArea*> GetAreas() {
		std::vector<DamageArea*> result;
		for (auto& a : areas_) {
			result.push_back(a.get());
		}
		return result;
	};

private:
	std::vector<std::unique_ptr<DamageArea>> areas_;
};

