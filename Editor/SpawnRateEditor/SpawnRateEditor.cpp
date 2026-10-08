#include "SpawnRateEditor.h"

SpawnRateEditor::SpawnRateEditor() {
	Load(filePath_);
}

void SpawnRateEditor::Draw() {
#ifdef USE_IMGUI
	ImGui::Begin("出現率エディタ");

	char buffer[512];
	std::snprintf(
		buffer,
		sizeof(buffer),
		"%s",
		filePath_.c_str()
	);

	// ファイル名設定
	if (ImGui::InputText("ファイル",buffer,sizeof(buffer))) {
		filePath_ = buffer;
	}

	// ロード
	if (ImGui::Button("ロード")) {
		Load(filePath_);
	}
	ImGui::SameLine();

	// セーブ
	if (ImGui::Button("セーブ")) {
		Save(filePath_);
	}
	ImGui::Separator();

	for (size_t i = 0; i < spawnRates_.size(); ++i) {
		SpawnRate& spawnRate = spawnRates_[i];

		ImGui::PushID(static_cast<int>(i));

		// 敵タイプ
		int enemyType = static_cast<int>(spawnRate.enemyType);
		if (ImGui::InputInt("敵タイプ", &enemyType)) {
			if (enemyType < 0) {
				enemyType = 0;
			}

			spawnRate.enemyType = static_cast<uint32_t>(enemyType);
		}

		// 重み
		ImGui::InputFloat("重み(出現率)",&spawnRate.weight);
		if (spawnRate.weight < 0.0f) {
			spawnRate.weight = 0.0f;
		}

		// 削除
		if (ImGui::Button("削除")) {
			RemoveSpawnRate(i);

			ImGui::PopID();

			--i;

			continue;
		}

		ImGui::Separator();

		ImGui::PopID();
	}

	// 追加
	if (ImGui::Button("追加")) {
		AddSpawnRate();
	}

	// 合計重み
	float totalWeight = 0.0f;
	for (const auto& spawnRate : spawnRates_) {
		totalWeight += spawnRate.weight;
	}

	ImGui::Separator();

	ImGui::Text("合計重み : %.2f", totalWeight);

	ImGui::End();
#endif
}

void SpawnRateEditor::AddSpawnRate() {
	SpawnRate spawnRate;

	spawnRate.enemyType = 0;
	spawnRate.weight = 0.0f;

	spawnRates_.push_back(spawnRate);
}

void SpawnRateEditor::RemoveSpawnRate(size_t index) {
	if (index >= spawnRates_.size()) {
		return;
	}

	spawnRates_.erase(spawnRates_.begin() + index);
}

void SpawnRateEditor::Save(const std::string& filePath) {
	nlohmann::json root;

	root["spawnRates"] = nlohmann::json::array();

	for (const auto& spawnRate : spawnRates_) {
		nlohmann::json data;

		data["enemyType"] = spawnRate.enemyType;
		data["weight"] = spawnRate.weight;
		root["spawnRates"].push_back(data);
	}

	std::ofstream file(filePath);

	if (!file.is_open()) {
		return;
	}

	file << root.dump(4);
}

void SpawnRateEditor::Load(const std::string& filePath) {
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

	spawnRates_.clear();

	for (const auto& data : root["spawnRates"]) {
		SpawnRate spawnRate;

		if (data.contains("enemyType")) {
			spawnRate.enemyType = data["enemyType"].get<uint32_t>();
		}
		if (data.contains("weight")) {
			spawnRate.weight = data["weight"].get<float>();
		}

		spawnRates_.push_back(spawnRate);
	}
}