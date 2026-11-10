#include "EnemySpawnSetting.h"
#include <Externals/nlohmann/json.hpp>
#include <fstream>

EnemySpawnSetting::EnemySpawnSetting() {
}

void EnemySpawnSetting::Load(std::string filePath) {
	std::ifstream file(filePath);

	if (!file.is_open()) {
		return;
	}

	nlohmann::json root;

	try {
		file >> root;
	} catch (...) {
		return;
	}

	if (!root.contains("spawnRates")) {
		return;
	}

	if (!root["spawnRates"].is_array()) {
		return;
	}

	spawnData_.clear();

	for (const auto& data : root["spawnRates"]) {
		EnemySpawn spawnRate;

		if (data.contains("name")) {
			spawnRate.name = data["name"].get<std::string>();
		}
		if (data.contains("weight")) {
			spawnRate.weight = data["weight"].get<float>();
		}

		spawnData_.push_back(spawnRate);
	}
}
