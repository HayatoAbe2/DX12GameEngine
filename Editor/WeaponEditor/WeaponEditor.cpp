#include "WeaponEditor.h"
#include "Data/WeaponSerializer.h"
#include "Editor/EditorFile.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>

#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_stdlib.h"
#include "Editor/EditorUtil.h"
#endif

namespace {
	constexpr const char* kWeaponDir = "Resources/Data/Weapons";
	constexpr const char* kModelDir = "Resources/Models/Weapons";
	constexpr const char* kEnemySuffix = "_Enemy";

	// 名前に使える文字(ファイル名とjsonのキーを兼ねるため)
	bool IsValidName(const std::string& name) {
		if (name.empty()) { return false; }
		return std::all_of(name.begin(), name.end(), [](unsigned char c) {
			return std::isalnum(c) || c == '_';
		});
	}
}

WeaponEditor::WeaponEditor() {
	Scan();
}

std::string WeaponEditor::Snapshot(const WeaponData& weapon) {
	return WeaponSerializer::ToJson(weapon).dump();
}

std::string WeaponEditor::MakeKey(const std::string& name, bool isEnemy) {
	return isEnemy ? name + kEnemySuffix : name;
}

WeaponEditor::Entry* WeaponEditor::Selected() {
	if (selected_ < 0 || selected_ >= int(entries_.size())) { return nullptr; }
	return &entries_[selected_];
}

void WeaponEditor::Scan() {
	entries_.clear();
	selected_ = -1;
	models_ = EditorFile::ListFiles(kModelDir, { ".obj", ".glb", ".gltf" });

	for (const std::string& fileName : EditorFile::ListFiles(kWeaponDir, { ".json" })) {
		const std::string path = (std::filesystem::path(kWeaponDir) / fileName).generic_string();

		nlohmann::ordered_json root;
		if (!EditorFile::ReadJson(path, root) || !root.is_object()) {
			status_ = "読み込めないファイルがあります: " + path;
			continue;
		}

		for (const auto& [key, value] : root.items()) {
			if (!value.is_object()) { continue; }
			Entry entry;
			entry.key = key;
			entry.path = path;
			entry.data = WeaponSerializer::FromJson(key, nlohmann::json(value));
			entry.history.Reset(Snapshot(entry.data));
			entries_.push_back(std::move(entry));
		}
	}
	SortEntries();
}

void WeaponEditor::SortEntries() {
	const std::string selectedKey = Selected() ? Selected()->key : std::string();

	// プレイヤー用 → 敵用、その中は名前順
	std::stable_sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
		const bool ea = WeaponSerializer::IsEnemyWeapon(a.key);
		const bool eb = WeaponSerializer::IsEnemyWeapon(b.key);
		if (ea != eb) { return !ea; }
		return a.key < b.key;
	});

	selected_ = -1;
	for (int i = 0; i < int(entries_.size()); ++i) {
		if (entries_[i].key == selectedKey) { selected_ = i; }
	}
}

std::string WeaponEditor::CheckNewKey(const std::string& key, bool isEnemy) const {
	const std::string name = isEnemy && WeaponSerializer::IsEnemyWeapon(key)
		? key.substr(0, key.size() - std::string(kEnemySuffix).size()) : key;
	if (name.empty()) { return "名前を入力してください"; }
	if (!IsValidName(name)) { return "名前には英数字と _ だけが使えます"; }
	if (!isEnemy && WeaponSerializer::IsEnemyWeapon(key)) { return "末尾が _Enemy の名前は敵用になります(敵用にチェックしてください)"; }
	for (const auto& e : entries_) {
		if (e.key == key) { return "同じ名前の武器があります"; }
	}
	return {};
}

void WeaponEditor::AddNew(const std::string& key) {
	Entry entry;
	entry.key = key;
	entry.path = (std::filesystem::path(kWeaponDir) / (key + ".json")).generic_string();
	entry.data = WeaponSerializer::FromJson(key, nlohmann::json::object());
	if (!models_.empty()) { entry.data.modelName = models_.front(); }
	entry.history.Reset(Snapshot(entry.data));
	entry.isNew = true;

	entries_.push_back(std::move(entry));
	selected_ = int(entries_.size()) - 1;
	SortEntries();
}

void WeaponEditor::Duplicate() {
	const Entry* source = Selected();
	if (!source) { return; }

	const bool isEnemy = WeaponSerializer::IsEnemyWeapon(source->key);
	const std::string base = isEnemy ? source->key.substr(0, source->key.size() - std::string(kEnemySuffix).size()) : source->key;

	// 空いている名前を探す
	std::string key;
	for (int n = 1; n < 1000; ++n) {
		key = MakeKey(base + "_Copy" + (n > 1 ? std::to_string(n) : ""), isEnemy);
		if (CheckNewKey(key, isEnemy).empty()) { break; }
	}

	Entry entry;
	entry.key = key;
	entry.path = (std::filesystem::path(kWeaponDir) / (key + ".json")).generic_string();
	entry.data = source->data;
	entry.data.name = key;
	entry.history.Reset(Snapshot(entry.data));
	entry.isNew = true;

	entries_.push_back(std::move(entry));
	selected_ = int(entries_.size()) - 1;
	SortEntries();
}

void WeaponEditor::DeleteSelected() {
	Entry* entry = Selected();
	if (!entry) { return; }

	if (!entry->isNew) {
		nlohmann::ordered_json root;
		if (!EditorFile::ReadJson(entry->path, root)) {
			status_ = "削除できませんでした(ファイルを読めません): " + entry->path;
			return;
		}
		root.erase(entry->key);
		if (root.empty()) {
			std::error_code ec;
			std::filesystem::remove(entry->path, ec);
		} else {
			EditorFile::WriteJson(entry->path, root);
		}
		if (onApplied_) { onApplied_(); }
	}

	status_ = "削除しました: " + entry->key;
	entries_.erase(entries_.begin() + selected_);
	selected_ = -1;
}

bool WeaponEditor::Save(Entry& entry) {
	// 他の武器が同じファイルにあっても残すため、いったん読み込んでから差し替える
	nlohmann::ordered_json root;
	if (!EditorFile::ReadJson(entry.path, root)) {
		status_ = "保存できませんでした(既存のファイルを読めません): " + entry.path;
		return false;
	}
	if (!root.is_object()) { root = nlohmann::ordered_json::object(); }

	root[entry.key] = WeaponSerializer::ToJson(entry.data);
	if (!EditorFile::WriteJson(entry.path, root)) {
		status_ = "保存できませんでした(書き込めません): " + entry.path;
		return false;
	}

	entry.isNew = false;
	entry.history.MarkSaved();
	status_ = "保存しました: " + entry.path;
	if (onApplied_) { onApplied_(); }
	return true;
}

void WeaponEditor::ApplySnapshot(Entry& entry, const std::string& snapshot) {
	try {
		entry.data = WeaponSerializer::FromJson(entry.key, nlohmann::json::parse(snapshot));
	} catch (...) {}
}

void WeaponEditor::Undo(Entry& entry) {
	if (auto snapshot = entry.history.Undo()) { ApplySnapshot(entry, *snapshot); }
}

void WeaponEditor::Redo(Entry& entry) {
	if (auto snapshot = entry.history.Redo()) { ApplySnapshot(entry, *snapshot); }
}

// ----------------------------------------------------------------
// 画面
// ----------------------------------------------------------------

void WeaponEditor::Draw() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSize(ImVec2(920.0f, 680.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("武器エディタ")) {
		ImGui::End();
		return;
	}

	ImGui::BeginChild("list", ImVec2(230.0f, 0.0f), ImGuiChildFlags_Borders);
	DrawList();
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("editor", ImVec2(0.0f, 0.0f));
	if (Entry* entry = Selected()) {
		DrawEditor(*entry);
	} else {
		ImGui::TextDisabled("左の一覧から武器を選んでください");
	}
	ImGui::EndChild();

	DrawPopups();
	ImGui::End();
#endif
}

void WeaponEditor::DrawList() {
#ifdef USE_IMGUI
	if (ImGui::Button("新規追加")) { openNewPopup_ = true; }
	ImGui::SameLine();
	ImGui::BeginDisabled(Selected() == nullptr);
	if (ImGui::Button("コピー")) { Duplicate(); }
	ImGui::SameLine();
	if (ImGui::Button("削除")) { openDeletePopup_ = true; }
	ImGui::EndDisabled();

	if (ImGui::Button("再読み込み")) {
		const bool anyDirty = std::any_of(entries_.begin(), entries_.end(), [](const Entry& e) { return e.history.dirty || e.isNew; });
		if (anyDirty) { openReloadPopup_ = true; } else { Scan(); }
	}
	ImGui::Separator();

	for (int group = 0; group < 2; ++group) {
		ImGui::SeparatorText(group == 0 ? "プレイヤー用" : "敵用");
		for (int i = 0; i < int(entries_.size()); ++i) {
			const Entry& e = entries_[i];
			if (WeaponSerializer::IsEnemyWeapon(e.key) != (group == 1)) { continue; }

			// 変更ありは * を付ける(### から後ろだけがIDになる)
			const std::string label = e.key + ((e.history.dirty || e.isNew) ? " *" : "") + "###" + e.key;
			if (ImGui::Selectable(label.c_str(), selected_ == i)) { selected_ = i; }
		}
	}
#endif
}

void WeaponEditor::DrawPopups() {
#ifdef USE_IMGUI
	if (openNewPopup_) { ImGui::OpenPopup("新規武器"); openNewPopup_ = false; newName_.clear(); newIsEnemy_ = false; }
	if (openDeletePopup_) { ImGui::OpenPopup("武器を削除"); openDeletePopup_ = false; }
	if (openReloadPopup_) { ImGui::OpenPopup("再読込の確認"); openReloadPopup_ = false; }

	if (ImGui::BeginPopupModal("新規武器", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("名前", &newName_);
		ImGui::Checkbox("敵用(名前の末尾に _Enemy を付けます)", &newIsEnemy_);

		const std::string key = MakeKey(newName_, newIsEnemy_);
		const std::string problem = CheckNewKey(key, newIsEnemy_);
		if (problem.empty()) {
			ImGui::TextDisabled("保存先: %s/%s.json", kWeaponDir, key.c_str());
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", problem.c_str());
		}

		ImGui::BeginDisabled(!problem.empty());
		if (ImGui::Button("作成")) {
			AddNew(key);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("武器を削除", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (const Entry* e = Selected()) {
			ImGui::Text("「%s」を削除します。", e->key.c_str());
			if (!e->isNew) { ImGui::TextDisabled("ファイル(%s)から取り除かれます。", e->path.c_str()); }
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

void WeaponEditor::DrawEditor(Entry& entry) {
#ifdef USE_IMGUI
	WeaponData& w = entry.data;
	const bool isEnemy = WeaponSerializer::IsEnemyWeapon(entry.key);

	// Ctrl+Z / Ctrl+Y(文字入力中は除く)
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive()) {
		const ImGuiIO& io = ImGui::GetIO();
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
			if (io.KeyShift) { Redo(entry); } else { Undo(entry); }
		} else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
			Redo(entry);
		}
	}

	// ---- ヘッダ ----
	ImGui::Text("%s", entry.key.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("%s%s", entry.path.c_str(), entry.isNew ? "(未保存の新規)" : "");

	const std::vector<DataIssue> issues = WeaponSerializer::Validate(w);
	const bool hasError = std::any_of(issues.begin(), issues.end(), [](const DataIssue& i) { return i.isError; });

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

	// ---- テスト用:この武器をプレイヤーの足元に出す ----
	if (onSpawn_) {
		const bool needSave = entry.history.dirty || entry.isNew;
		ImGui::BeginDisabled(hasError || isEnemy);
		if (ImGui::Button(needSave ? "保存してプレイヤーの足元に出す" : "プレイヤーの足元に出す")) {
			// 保存すると武器データが再読み込みされ、編集した内容で出る
			if (!needSave || Save(entry)) { onSpawn_(entry.key); }
		}
		ImGui::EndDisabled();
		if (isEnemy && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip("敵用の武器は拾えません。敵エディタで、この武器を持つ敵を出してください");
		}
	}

	if (!status_.empty()) { ImGui::TextDisabled("%s", status_.c_str()); }
	EditorUtil::DrawIssues(issues);
	ImGui::Separator();

	// ---- 基本 ----
	if (ImGui::CollapsingHeader("基本", ImGuiTreeNodeFlags_DefaultOpen)) {
		// モデル
		const char* preview = w.modelName.empty() ? "(未選択)" : w.modelName.c_str();
		if (ImGui::BeginCombo("モデル", preview)) {
			for (const std::string& model : models_) {
				const bool selected = (model == w.modelName);
				if (ImGui::Selectable(model.c_str(), selected)) { w.modelName = model; }
				if (selected) { ImGui::SetItemDefaultFocus(); }
			}
			ImGui::EndCombo();
		}
		if (!w.modelName.empty() && std::find(models_.begin(), models_.end(), w.modelName) == models_.end()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s にモデルが見つかりません", kModelDir);
		}

		// レア度
		static const char* kRarityNames[] = { "Common", "Rare", "Epic", "Legendary" };
		int rarity = std::clamp(int(w.rarity) - 1, 0, 3);
		if (ImGui::Combo("レア度", &rarity, kRarityNames, 4)) { w.rarity = Rarity(rarity + 1); }

		EditorUtil::DragFloat("重さ(射撃中の減速)", &w.stats.weight, 0.01f, 0.0f, 1.0f);
		EditorUtil::DragFloat("クールタイム(秒)", &w.stats.shootCoolTime, 0.01f, 0.0f, 30.0f);
		if (!isEnemy) {
			EditorUtil::DragFloat("最大チャージ(弾数)", &w.stats.maxCharge, 0.1f, 0.0f, 100.0f, "%.1f");
			EditorUtil::DragFloat("1発分のチャージ時間(秒)", &w.stats.chargeTime, 0.01f, 0.0f, 30.0f);
		}
	}

	// ---- 弾 ----
	if (ImGui::CollapsingHeader("弾", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& b = w.bullet;
		EditorUtil::DragFloat("ダメージ", &b.damage, 0.1f, -1000.0f, 10000.0f, "%.1f");
		EditorUtil::DragFloat("大きさ(半径)", &b.radius, 0.01f, 0.0f, 20.0f);
		EditorUtil::DragFloat("弾速", &b.speed, 0.1f, 0.0f, 500.0f, "%.1f");
		EditorUtil::DragInt("寿命(フレーム)", &b.lifeTime, 1.0f, 0, 100000);
		ImGui::SameLine();
		ImGui::TextDisabled("= %.2f秒", float(b.lifeTime) / 60.0f);
		EditorUtil::DragFloat("ノックバック", &b.knockback, 0.01f, 0.0f, 20.0f);

		float color[4] = { b.color.x, b.color.y, b.color.z, b.color.w };
		if (ImGui::ColorEdit4("色", color)) {
			b.color = { color[0], color[1], color[2], color[3] };
		}
	}

	// ---- 武器の特性 ----
	if (ImGui::CollapsingHeader("武器の特性", ImGuiTreeNodeFlags_DefaultOpen)) {
		MultiShotParam multiDefault{};
		multiDefault.count = 3;
		multiDefault.maxAngle = 30.0f;
		multiDefault.random = false;
		EditorUtil::Section("同時発射(multiShot)", w.traits.multiShot, multiDefault, [](MultiShotParam& m) {
			int count = m.count;
			if (EditorUtil::DragInt("発射数", &count, 0.1f, 1, 60)) { m.count = static_cast<int8_t>(count); }
			EditorUtil::DragFloat("拡散角度(度)", &m.maxAngle, 0.5f, 0.0f, 360.0f, "%.1f");
			ImGui::Checkbox("ランダム(未実装)", &m.random);
		});

		ChargeParam chargeDefault{};
		chargeDefault.time = 1.0f;
		chargeDefault.damage = 5.0f;
		EditorUtil::Section("溜め射撃(charge)", w.traits.charge, chargeDefault, [](ChargeParam& c) {
			EditorUtil::DragFloat("最大までの時間(秒)", &c.time, 0.01f, 0.0f, 30.0f);
			EditorUtil::DragFloat("最大時の追加ダメージ", &c.damage, 0.1f, 0.0f, 10000.0f, "%.1f");
		});

		BurstParam burstDefault{};
		burstDefault.count = 3;
		burstDefault.interval = 0.1f;
		EditorUtil::Section("バースト(burst)", w.traits.burst, burstDefault, [](BurstParam& bu) {
			EditorUtil::DragInt("連続回数", &bu.count, 0.1f, 1, 60);
			EditorUtil::DragFloat("間隔(秒)", &bu.interval, 0.005f, 0.0f, 5.0f, "%.3f");
		});
	}

	// ---- 弾の性質 ----
	if (ImGui::CollapsingHeader("弾の性質", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& t = w.bullet.traits;

		ImGui::SeparatorText("移動");
		Orbit orbitDefault{};
		orbitDefault.radius = 3.0f;
		orbitDefault.speed = 2.0f;
		EditorUtil::Section("軌道(orbit)", t.move.orbit, orbitDefault, [](Orbit& o) {
			EditorUtil::DragFloat("半径", &o.radius, 0.05f, 0.0f, 50.0f);
			EditorUtil::DragFloat("回転速度", &o.speed, 0.05f, -50.0f, 50.0f);
		});
		Wave waveDefault{};
		waveDefault.amplitude = 1.0f;
		waveDefault.speed = 1.0f;
		waveDefault.maxAngle = 30.0f;
		EditorUtil::Section("蛇行(wave)", t.move.wave, waveDefault, [](Wave& v) {
			EditorUtil::DragFloat("振幅", &v.amplitude, 0.05f, 0.0f, 50.0f);
			EditorUtil::DragFloat("速さ", &v.speed, 0.05f, 0.0f, 50.0f);
			EditorUtil::DragFloat("最大角度(度)", &v.maxAngle, 0.5f, 0.0f, 180.0f, "%.1f");
		});
		Accel accelDefault{};
		accelDefault.rate = 1.02f;
		EditorUtil::Section("加速(accel)", t.move.accel, accelDefault, [](Accel& a) {
			EditorUtil::DragFloat("加速率(毎フレーム掛ける)", &a.rate, 0.001f, 0.0f, 2.0f, "%.3f");
		});

		ImGui::SeparatorText("何かに当たったとき");
		Explode explodeDefault{};
		explodeDefault.radius = 3.0f;
		explodeDefault.damage = w.bullet.damage;
		EditorUtil::Section("爆発(explode)", t.onHitAnything.explode, explodeDefault, [](Explode& e) {
			EditorUtil::DragFloat("半径", &e.radius, 0.05f, 0.0f, 50.0f);
			EditorUtil::DragFloat("ダメージ", &e.damage, 0.1f, 0.0f, 10000.0f, "%.1f");
		});

		ImGui::SeparatorText("壁に当たったとき");
		Ricochet ricochetDefault{};
		ricochetDefault.count = 2;
		ricochetDefault.onHitEnemy = false;
		ricochetDefault.toEnemy = false;
		EditorUtil::Section("跳弾(ricochet)", t.onHitWall.ricochet, ricochetDefault, [](Ricochet& r) {
			EditorUtil::DragInt("回数", &r.count, 0.1f, 0, 100);
		});

		ImGui::SeparatorText("敵に当たったとき");
		Piercing piercingDefault{};
		piercingDefault.count = 1;
		EditorUtil::Section("貫通(piercing)", t.onHitEnemy.piercing, piercingDefault, [](Piercing& p) {
			EditorUtil::DragInt("貫通回数", &p.count, 0.1f, 0, 100);
		});
		Slow slowDefault{};
		slowDefault.rate = 50.0f;
		slowDefault.time = 2.0f;
		EditorUtil::Section("減速(slow)", t.onHitEnemy.slow, slowDefault, [](Slow& s) {
			ImGui::TextDisabled("減速の割合と時間は、ゲーム側で固定値(5秒・0.4倍)です");
			EditorUtil::DragFloat("割合(%)(未反映)", &s.rate, 0.5f, 0.0f, 100.0f, "%.1f");
			EditorUtil::DragFloat("時間(秒)(未反映)", &s.time, 0.05f, 0.0f, 60.0f);
		});
	}

	// ---- 目安 ----
	if (ImGui::CollapsingHeader("目安とプレビュー", ImGuiTreeNodeFlags_DefaultOpen)) {
		DrawPreview(w);
	}

	// 操作が終わったタイミングで履歴に積む(ドラッグ・入力中は積まない)
	if (!ImGui::IsAnyItemActive()) {
		entry.history.Commit(Snapshot(w));
	}
#endif
}

void WeaponEditor::DrawPreview(const WeaponData& w) {
#ifdef USE_IMGUI
	const float range = WeaponSerializer::EstimateRange(w);
	if (range >= WeaponSerializer::kMaxRange) {
		ImGui::Text("射程(目安): 壁に当たるまで");
	} else {
		ImGui::Text("射程(目安): %.1f", range);
	}

	// 1回の射撃で出る弾数とダメージ
	const int multi = w.traits.multiShot ? (std::max)(1, int(w.traits.multiShot->count)) : 1;
	const int burst = w.traits.burst ? (std::max)(1, w.traits.burst->count) : 1;
	const float damagePerTrigger = w.bullet.damage * float(multi * burst);
	ImGui::Text("1回の射撃: %d発 / ダメージ合計 %.1f", multi * burst, damagePerTrigger);
	if (w.stats.shootCoolTime > 0.0f) {
		ImGui::Text("毎秒ダメージ(目安・全弾命中): %.1f", damagePerTrigger / w.stats.shootCoolTime);
	}
	if (w.traits.burst) {
		ImGui::Text("バーストの所要時間: %.2f秒", float((std::max)(w.traits.burst->count - 1, 0)) * w.traits.burst->interval);
	}
	if (!WeaponSerializer::IsEnemyWeapon(w.name)) {
		ImGui::Text("連射: 満タン %.1f発 / 1発回復 %.2f秒", w.stats.maxCharge, w.stats.chargeTime);
	}

	// 真上から見た射撃の広がり(上が前方)
	const ImVec2 size(260.0f, 170.0f);
	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImGui::Dummy(size);
	ImDrawList* draw = ImGui::GetWindowDrawList();
	draw->AddRectFilled(p0, ImVec2(p0.x + size.x, p0.y + size.y), IM_COL32(24, 24, 28, 255));
	draw->AddRect(p0, ImVec2(p0.x + size.x, p0.y + size.y), IM_COL32(90, 90, 100, 255));

	const ImVec2 origin(p0.x + size.x * 0.5f, p0.y + size.y - 12.0f);
	const float drawRange = (std::min)(range, 60.0f);
	const float scale = (drawRange > 0.001f) ? (size.y - 30.0f) / drawRange : 1.0f;
	const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(w.bullet.color.x, w.bullet.color.y, w.bullet.color.z, 1.0f));

	if (w.bullet.traits.move.orbit) {
		// 軌道弾は自分を中心に回る
		draw->AddCircle(ImVec2(origin.x, origin.y - 40.0f), w.bullet.traits.move.orbit->radius * scale, color, 48);
	} else {
		const int lines = (w.traits.multiShot && !w.traits.multiShot->random) ? (std::max)(1, int(w.traits.multiShot->count)) : 1;
		const float maxAngle = (w.traits.multiShot ? w.traits.multiShot->maxAngle : 0.0f) * 3.14159265f / 180.0f;
		for (int i = 0; i < lines; ++i) {
			const float angle = (lines > 1) ? -maxAngle * 0.5f + maxAngle * float(i) / float(lines - 1) : 0.0f;
			const ImVec2 end(origin.x + std::sin(angle) * drawRange * scale, origin.y - std::cos(angle) * drawRange * scale);
			draw->AddLine(origin, end, color, 1.5f);
			draw->AddCircle(end, (std::max)(2.0f, w.bullet.radius * scale), color);
		}
	}
	draw->AddCircleFilled(origin, 3.0f, IM_COL32(255, 255, 255, 255));
	if (range > 60.0f) {
		ImGui::TextDisabled("(射程が長いため、図は60までを表示しています)");
	}
#endif
}
