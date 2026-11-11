#include "EnemyEditor.h"
#include "Data/EnemySerializer.h"
#include "Data/WeaponSerializer.h"
#include "Editor/EditorFile.h"
#include <algorithm>
#include <filesystem>
#include <format>

#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_stdlib.h"
#include "Editor/EditorUtil.h"
#endif

namespace {
	constexpr const char* kEnemyFile = "Resources/Data/Enemies/Enemies.json";
	constexpr const char* kModelDir = "Resources/Models/Enemy";

	// 名前に使える文字(jsonのキーと、出現率ファイルの name に使うため)
	bool IsValidName(const std::string& name) {
		if (name.empty()) { return false; }
		return std::all_of(name.begin(), name.end(), [](unsigned char c) {
			return std::isalnum(c) || c == '_';
		});
	}
}

EnemyEditor::EnemyEditor() {
	Scan();
}

std::string EnemyEditor::Snapshot(const EnemyData& data) {
	return EnemySerializer::ToJson(data).dump();
}

EnemyEditor::Entry* EnemyEditor::Selected() {
	if (selected_ < 0 || selected_ >= int(entries_.size())) { return nullptr; }
	return &entries_[selected_];
}

void EnemyEditor::Scan() {
	const std::string selectedKey = Selected() ? Selected()->key : std::string();
	entries_.clear();
	selected_ = -1;
	models_ = EditorFile::ListFiles(kModelDir, { ".obj", ".glb", ".gltf" });

	nlohmann::ordered_json root;
	if (!EditorFile::ReadJson(kEnemyFile, root) || !root.is_object()) {
		status_ = std::string("読み込めませんでした: ") + kEnemyFile;
		return;
	}

	for (const auto& [key, value] : root.items()) {
		if (!value.is_object()) { continue; }
		Entry entry;
		entry.key = key;
		entry.data = EnemySerializer::FromJson(nlohmann::json(value));
		entry.history.Reset(Snapshot(entry.data));
		entries_.push_back(std::move(entry));
	}

	for (int i = 0; i < int(entries_.size()); ++i) {
		if (entries_[i].key == selectedKey) { selected_ = i; }
	}
}

std::string EnemyEditor::CheckNewKey(const std::string& key) const {
	if (key.empty()) { return "名前を入力してください"; }
	if (!IsValidName(key)) { return "名前には英数字と _ だけが使えます"; }
	for (const auto& e : entries_) {
		if (e.key == key) { return "同じ名前の敵がいます"; }
	}
	return {};
}

void EnemyEditor::AddNew(const std::string& key) {
	Entry entry;
	entry.key = key;
	entry.data = EnemySerializer::FromJson(nlohmann::json::object());
	if (!models_.empty()) { entry.data.modelName = models_.front(); }
	entry.history.Reset(Snapshot(entry.data));
	entry.isNew = true;

	entries_.push_back(std::move(entry));
	selected_ = int(entries_.size()) - 1;
}

void EnemyEditor::Duplicate() {
	const Entry* source = Selected();
	if (!source) { return; }

	std::string key;
	for (int n = 1; n < 1000; ++n) {
		key = source->key + "_Copy" + (n > 1 ? std::to_string(n) : "");
		if (CheckNewKey(key).empty()) { break; }
	}

	Entry entry;
	entry.key = key;
	entry.data = source->data;
	entry.history.Reset(Snapshot(entry.data));
	entry.isNew = true;

	entries_.push_back(std::move(entry));
	selected_ = int(entries_.size()) - 1;
}

void EnemyEditor::DeleteSelected() {
	Entry* entry = Selected();
	if (!entry) { return; }

	if (!entry->isNew) {
		nlohmann::ordered_json root;
		if (!EditorFile::ReadJson(kEnemyFile, root)) {
			status_ = std::string("削除できませんでした(ファイルを読めません): ") + kEnemyFile;
			return;
		}
		root.erase(entry->key);
		if (!EditorFile::WriteJson(kEnemyFile, root)) {
			status_ = std::string("削除できませんでした(書き込めません): ") + kEnemyFile;
			return;
		}
		if (onApplied_) { onApplied_(); }
	}

	status_ = "削除しました: " + entry->key;
	entries_.erase(entries_.begin() + selected_);
	selected_ = -1;
}

bool EnemyEditor::Save(Entry& entry) {
	// 他の敵はそのまま残すため、いったん読み込んでから差し替える(新規は末尾に追加される)
	nlohmann::ordered_json root;
	if (!EditorFile::ReadJson(kEnemyFile, root)) {
		status_ = std::string("保存できませんでした(既存のファイルを読めません): ") + kEnemyFile;
		return false;
	}
	if (!root.is_object()) { root = nlohmann::ordered_json::object(); }

	root[entry.key] = EnemySerializer::ToJson(entry.data);
	if (!EditorFile::WriteJson(kEnemyFile, root)) {
		status_ = std::string("保存できませんでした(書き込めません): ") + kEnemyFile;
		return false;
	}

	entry.isNew = false;
	entry.history.MarkSaved();
	status_ = std::string("保存しました: ") + kEnemyFile;
	if (onApplied_) { onApplied_(); }
	return true;
}

void EnemyEditor::ApplySnapshot(Entry& entry, const std::string& snapshot) {
	try {
		entry.data = EnemySerializer::FromJson(nlohmann::json::parse(snapshot));
	} catch (...) {}
}

void EnemyEditor::Undo(Entry& entry) {
	if (auto snapshot = entry.history.Undo()) { ApplySnapshot(entry, *snapshot); }
}

void EnemyEditor::Redo(Entry& entry) {
	if (auto snapshot = entry.history.Redo()) { ApplySnapshot(entry, *snapshot); }
}

// ----------------------------------------------------------------
// 画面
// ----------------------------------------------------------------

void EnemyEditor::Draw(const std::vector<std::string>& enemyWeaponNames, const WeaponLookup& findWeapon) {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSize(ImVec2(860.0f, 640.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("敵エディタ")) {
		ImGui::End();
		return;
	}

	ImGui::BeginChild("list", ImVec2(210.0f, 0.0f), ImGuiChildFlags_Borders);
	DrawList();
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("editor", ImVec2(0.0f, 0.0f));
	if (Entry* entry = Selected()) {
		DrawEditor(*entry, enemyWeaponNames, findWeapon);
	} else {
		ImGui::TextDisabled("左の一覧から敵を選んでください");
	}
	ImGui::EndChild();

	DrawPopups();
	ImGui::End();
#endif
}

void EnemyEditor::DrawList() {
#ifdef USE_IMGUI
	if (ImGui::Button("新規")) { openNewPopup_ = true; }
	ImGui::SameLine();
	ImGui::BeginDisabled(Selected() == nullptr);
	if (ImGui::Button("複製")) { Duplicate(); }
	ImGui::SameLine();
	if (ImGui::Button("削除")) { openDeletePopup_ = true; }
	ImGui::EndDisabled();

	if (ImGui::Button("再読込")) {
		const bool anyDirty = std::any_of(entries_.begin(), entries_.end(), [](const Entry& e) { return e.history.dirty || e.isNew; });
		if (anyDirty) { openReloadPopup_ = true; } else { Scan(); }
	}
	ImGui::Separator();

	for (int i = 0; i < int(entries_.size()); ++i) {
		const Entry& e = entries_[i];
		const std::string label = e.key + ((e.history.dirty || e.isNew) ? " *" : "") + "###" + e.key;
		if (ImGui::Selectable(label.c_str(), selected_ == i)) { selected_ = i; }
	}
#endif
}

void EnemyEditor::DrawPopups() {
#ifdef USE_IMGUI
	if (openNewPopup_) { ImGui::OpenPopup("新規の敵"); openNewPopup_ = false; newName_.clear(); }
	if (openDeletePopup_) { ImGui::OpenPopup("敵を削除"); openDeletePopup_ = false; }
	if (openReloadPopup_) { ImGui::OpenPopup("再読込の確認"); openReloadPopup_ = false; }

	if (ImGui::BeginPopupModal("新規の敵", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("名前", &newName_);
		const std::string problem = CheckNewKey(newName_);
		if (!problem.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", problem.c_str());
		}

		ImGui::BeginDisabled(!problem.empty());
		if (ImGui::Button("作成")) {
			AddNew(newName_);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("敵を削除", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (const Entry* e = Selected()) {
			ImGui::Text("「%s」を削除します。", e->key.c_str());
			if (!e->isNew) { ImGui::TextDisabled("出現率のファイルがこの名前を使っていないか確認してください。"); }
		}
		if (ImGui::Button("削除する")) {
			DeleteSelected();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("再読込の確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("保存していない変更は破棄されます。");
		if (ImGui::Button("再読込する")) {
			Scan();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
#endif
}

void EnemyEditor::DrawEditor(Entry& entry, const std::vector<std::string>& weaponNames, const WeaponLookup& findWeapon) {
#ifdef USE_IMGUI
	EnemyData& d = entry.data;
	EnemyStatus& s = d.status;

	// Ctrl+Z / Ctrl+Y(文字入力中は除く)
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive()) {
		const ImGuiIO& io = ImGui::GetIO();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
			if (io.KeyShift) { Redo(entry); } else { Undo(entry); }
		} else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
			Redo(entry);
		}
	}

	// ---- 検証(敵単体 + 武器との照合) ----
	std::vector<DataIssue> issues = EnemySerializer::Validate(d);
	for (const std::string& weaponName : d.weapons) {
		const bool known = std::find(weaponNames.begin(), weaponNames.end(), weaponName) != weaponNames.end();
		if (!known) {
			issues.push_back({ true, std::format("武器「{}」が見つかりません", weaponName) });
			continue;
		}
		const WeaponData* weapon = findWeapon ? findWeapon(weaponName) : nullptr;
		if (weapon && s.attackRadius > 0.0f) {
			const float range = WeaponSerializer::EstimateRange(*weapon);
			if (range < s.attackRadius) {
				issues.push_back({ false, std::format("武器「{}」の射程(目安 {:.1f})が攻撃範囲({:.1f})より短く、届きません", weaponName, range, s.attackRadius) });
			}
		}
	}
	if (!d.weapons.empty() && s.attackRadius <= 0.0f) {
		issues.push_back({ false, "攻撃範囲が0以下のため、武器を持っていても攻撃しません" });
	}
	if (d.weapons.empty() && s.attackRadius > 0.0f) {
		issues.push_back({ false, "武器が無いため、攻撃範囲に入っても攻撃しません" });
	}
	if (d.weapons.size() > 1) {
		issues.push_back({ false, "複数の武器を持たせても、現在は先頭の武器しか使われません" });
	}
	const bool hasError = std::any_of(issues.begin(), issues.end(), [](const DataIssue& i) { return i.isError; });

	// ---- ヘッダ ----
	ImGui::Text("%s", entry.key.c_str());
	if (entry.isNew) { ImGui::SameLine(); ImGui::TextDisabled("(未保存の新規)"); }

	ImGui::BeginDisabled(hasError);
	if (ImGui::Button("保存して反映")) { Save(entry); }
	ImGui::EndDisabled();
	if (hasError && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip("エラーを直すと保存できます");
	}

	ImGui::SameLine();
	ImGui::BeginDisabled(entry.history.undo.empty());
	if (ImGui::Button("元に戻す")) { Undo(entry); }
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(entry.history.redo.empty());
	if (ImGui::Button("やり直す")) { Redo(entry); }
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (entry.history.dirty || entry.isNew) { ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "未保存の変更あり"); }

	// ---- テスト用:この敵をプレイヤーの近くに出す ----
	if (onSpawn_) {
		const bool needSave = entry.history.dirty || entry.isNew;
		ImGui::BeginDisabled(hasError);
		if (ImGui::Button(needSave ? "保存してプレイヤーの近くに出す" : "プレイヤーの近くに出す")) {
			// 保存すると敵データが再読み込みされ、編集した内容で出る
			if (!needSave || Save(entry)) { onSpawn_(entry.key); }
		}
		ImGui::EndDisabled();
	}

	if (!status_.empty()) { ImGui::TextDisabled("%s", status_.c_str()); }
	EditorUtil::DrawIssues(issues);
	ImGui::Separator();

	// ---- 見た目 ----
	if (ImGui::CollapsingHeader("見た目", ImGuiTreeNodeFlags_DefaultOpen)) {
		const char* preview = d.modelName.empty() ? "(未選択)" : d.modelName.c_str();
		if (ImGui::BeginCombo("モデル", preview)) {
			for (const std::string& model : models_) {
				const bool selected = (model == d.modelName);
				if (ImGui::Selectable(model.c_str(), selected)) { d.modelName = model; }
				if (selected) { ImGui::SetItemDefaultFocus(); }
			}
			ImGui::EndCombo();
		}
		if (!d.modelName.empty() && std::find(models_.begin(), models_.end(), d.modelName) == models_.end()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s にモデルが見つかりません", kModelDir);
		}
		EditorUtil::DragInt("アニメーション番号(-1でなし)", &d.animationIndex, 0.1f, -1, 64);
	}

	// ---- ステータス ----
	if (ImGui::CollapsingHeader("ステータス", ImGuiTreeNodeFlags_DefaultOpen)) {
		EditorUtil::DragFloat("HP", &s.hp, 0.5f, 0.0f, 100000.0f, "%.1f");
		EditorUtil::DragFloat("大きさ(半径)", &s.radius, 0.01f, 0.0f, 20.0f);
		EditorUtil::DragFloat("移動速度", &s.moveSpeed, 0.05f, 0.0f, 100.0f);
		EditorUtil::DragFloat("発見範囲", &s.defaultSearchRadius, 0.5f, 0.0f, 1000.0f, "%.1f");
		EditorUtil::DragFloat("攻撃範囲(0以下で攻撃しない)", &s.attackRadius, 0.1f, -1.0f, 1000.0f, "%.1f");
		EditorUtil::DragFloat("スタン耐性(10以上で怯まない)", &s.stunResist, 0.1f, 0.0f, 100.0f, "%.1f");
		ImGui::Checkbox("飛行", &s.canFly);

		ContactDamage contactDefault{};
		contactDefault.damage = 3.0f;
		EditorUtil::Section("接触ダメージ", s.traits.contactDamage, contactDefault, [](ContactDamage& c) {
			EditorUtil::DragFloat("ダメージ", &c.damage, 0.1f, 0.0f, 1000.0f, "%.1f");
		});
	}

	// ---- 出現数 ----
	if (ImGui::CollapsingHeader("出現数", ImGuiTreeNodeFlags_DefaultOpen)) {
		EditorUtil::DragInt("最小", &d.minSpawnCount, 0.1f, 1, 100);
		EditorUtil::DragInt("最大", &d.maxSpawnCount, 0.1f, 1, 100);
	}

	// ---- 武器 ----
	if (ImGui::CollapsingHeader("武器", ImGuiTreeNodeFlags_DefaultOpen)) {
		int removeIndex = -1;
		int swapIndex = -1;   // swapIndex と swapIndex+1 を入れ替える

		for (int i = 0; i < int(d.weapons.size()); ++i) {
			ImGui::PushID(i);
			std::string& current = d.weapons[i];

			ImGui::SetNextItemWidth(220.0f);
			if (ImGui::BeginCombo("##weapon", current.empty() ? "(未選択)" : current.c_str())) {
				for (const std::string& name : weaponNames) {
					const bool selected = (name == current);
					if (ImGui::Selectable(name.c_str(), selected)) { current = name; }
					if (selected) { ImGui::SetItemDefaultFocus(); }
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(i == 0);
			if (ImGui::ArrowButton("##up", ImGuiDir_Up)) { swapIndex = i - 1; }
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(i + 1 >= int(d.weapons.size()));
			if (ImGui::ArrowButton("##down", ImGuiDir_Down)) { swapIndex = i; }
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("削除")) { removeIndex = i; }

			// 武器の目安
			if (const WeaponData* weapon = findWeapon ? findWeapon(current) : nullptr) {
				const float range = WeaponSerializer::EstimateRange(*weapon);
				if (range >= WeaponSerializer::kMaxRange) {
					ImGui::TextDisabled("    射程(目安): 壁まで / ダメージ %.1f", weapon->bullet.damage);
				} else {
					ImGui::TextDisabled("    射程(目安): %.1f / ダメージ %.1f", range, weapon->bullet.damage);
				}
			}
			ImGui::PopID();
		}

		if (removeIndex >= 0) { d.weapons.erase(d.weapons.begin() + removeIndex); }
		if (swapIndex >= 0) { std::swap(d.weapons[swapIndex], d.weapons[swapIndex + 1]); }

		ImGui::BeginDisabled(weaponNames.empty());
		if (ImGui::Button("武器を追加")) { d.weapons.push_back(weaponNames.front()); }
		ImGui::EndDisabled();
		ImGui::TextDisabled("多段攻撃は、武器の「バースト」で表します(武器エディタ)");
	}

	// 操作が終わったタイミングで履歴に積む(ドラッグ・入力中は積まない)
	if (!ImGui::IsAnyItemActive()) {
		entry.history.Commit(Snapshot(d));
	}
#endif
}
