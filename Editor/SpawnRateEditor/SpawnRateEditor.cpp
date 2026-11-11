#include "SpawnRateEditor.h"

SpawnRateEditor::SpawnRateEditor() {
	Load(filePath_);
}

void SpawnRateEditor::Draw(const std::vector<std::string>& enemyNames) {
#ifdef USE_IMGUI
	ImGui::Begin("出現率");

	char buffer[512];
	std::snprintf(
		buffer,
		sizeof(buffer),
		"%s",
		filePath_.c_str()
	);

	// ファイル名設定
	if (ImGui::InputText("ファイル", buffer, sizeof(buffer))) {
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

	float totalWeight = 0.0f;
	for (const auto& r : spawnRates_) { totalWeight += r.weight; }

	for (size_t i = 0; i < spawnRates_.size(); ++i) {
		SpawnRate& spawnRate = spawnRates_[i];

		ImGui::PushID(static_cast<int>(i));

		// 敵の名前(登録済みの敵から選ぶ)
		const bool known = std::find(enemyNames.begin(), enemyNames.end(), spawnRate.name) != enemyNames.end();
		std::string preview = spawnRate.name.empty() ? "(未選択)" : spawnRate.name;
		if (ImGui::BeginCombo("敵", preview.c_str())) {
			for (const auto& name : enemyNames) {
				const bool selected = (name == spawnRate.name);
				if (ImGui::Selectable(name.c_str(), selected)) {
					spawnRate.name = name;
				}
				if (selected) { ImGui::SetItemDefaultFocus(); }
			}
			ImGui::EndCombo();
		}
		if (!spawnRate.name.empty() && !known) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Enemies.json に存在しない名前です");
		}

		// 重み
		ImGui::InputFloat("重み(出現率)", &spawnRate.weight);
		if (spawnRate.weight < 0.0f) { spawnRate.weight = 0.0f; }
		if (totalWeight > 0.0f) {
			ImGui::SameLine();
			ImGui::Text("(%.1f%%)", spawnRate.weight / totalWeight * 100.0f);
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

	ImGui::Separator();

	// 合計重み
	ImGui::Text("合計重み : %.2f", totalWeight);

	ImGui::End();
#endif
}

void SpawnRateEditor::AddSpawnRate() {
	SpawnRate spawnRate;

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

		data["name"] = spawnRate.name;
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

		if (data.contains("name")) {
			spawnRate.name = data["name"].get<std::string>();
		}
		if (data.contains("weight")) {
			spawnRate.weight = data["weight"].get<float>();
		}

		spawnRates_.push_back(spawnRate);
	}
}