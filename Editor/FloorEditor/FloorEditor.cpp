#include "Editor/FloorEditor/FloorEditor.h"
#include "Data/FloorConfigSerializer.h"
#include "Data/JsonFormat.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <set>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace {

// 部屋ファイルのフォルダ(FloorGenerator::LoadRoom と同じ)
constexpr const char* kRoomDir = "Resources/Debug/SceneEditor/";

} // namespace

FloorEditor::FloorEditor(const std::string& filePath) : path_(filePath) {
	Load();
	status_.clear();
}

void FloorEditor::Load() {
	FloorConfig loaded;
	if (!FloorConfigSerializer::Load(path_, loaded)) {
		status_ = "読み込めません(JSONが壊れています): " + path_;
		return;
	}
	config_ = std::move(loaded);
	savedText_ = Snapshot();
	status_ = "読み込みました";
}

bool FloorEditor::Save() {
	if (!FloorConfigSerializer::Save(path_, config_)) {
		status_ = "書き込めません: " + path_;
		return false;
	}

	// ファイルと同じく、固定の部屋を階層順にそろえる
	std::stable_sort(config_.fixedRooms.begin(), config_.fixedRooms.end(),
		[](const FixedRoom& a, const FixedRoom& b) { return a.depth < b.depth; });

	savedText_ = Snapshot();
	status_ = "保存しました。次の部屋から反映されます";
	if (onApplied_) { onApplied_(); }
	return true;
}

std::string FloorEditor::Snapshot() const {
	return DumpJson(FloorConfigSerializer::ToJson(config_));
}

std::vector<DataIssue> FloorEditor::CollectIssues() const {
	std::vector<DataIssue> issues = FloorConfigSerializer::Validate(config_);

	// 使われる可能性のある部屋タイプの、部屋ファイルがあるか
	std::set<int> used = { int(RoomType::Start), int(RoomType::Goal) };
	for (const auto& fixed : config_.fixedRooms) { used.insert(int(fixed.type)); }
	for (const auto& rate : config_.roomRates) {
		if (rate.weight > 0.0f) { used.insert(int(rate.type)); }
	}
	for (int t : used) {
		const std::string file = std::string(kRoomDir) + RoomFilePrefix(RoomType(t)) + "1.json";
		std::error_code ec;
		if (!std::filesystem::exists(file, ec)) {
			issues.push_back({ false, std::string(RoomTypeLabel(RoomType(t))) + "の部屋ファイルがありません: " + file });
		}
	}
	return issues;
}

void FloorEditor::AddFixedRoom() {
	// まだ固定されていない、いちばん小さい階層
	int depth = 1;
	for (int d = 1; d < config_.goalDepth; ++d) {
		if (!config_.FixedTypeAt(d)) { depth = d; break; }
	}
	config_.fixedRooms.push_back({ depth, RoomType::Shop });
}

#ifdef USE_IMGUI

namespace {

// 固定できる種類(スタートとゴールは移動回数で決まるので除く)
constexpr RoomType kFixedTypes[] = { RoomType::Combat, RoomType::Elite, RoomType::Boss, RoomType::Shop, RoomType::Event };

// 確率プレビューで選べる「直前の部屋」
constexpr RoomType kParentTypes[] = { RoomType::Start, RoomType::Combat, RoomType::Elite, RoomType::Shop, RoomType::Event, RoomType::Boss };

const ImVec4 kErrorColor(1.0f, 0.4f, 0.4f, 1.0f);
const ImVec4 kWarnColor(1.0f, 0.85f, 0.3f, 1.0f);
const ImVec4 kFixedColor(0.5f, 0.8f, 1.0f, 1.0f);

// 重みの入力1行。total が正なら、合計に対する割合を横に出す
void WeightRow(const char* label, float& value, float total) {
	ImGui::SetNextItemWidth(110.0f);
	ImGui::InputFloat(label, &value, 0.0f, 0.0f, "%.1f");
	if (value < 0.0f) { value = 0.0f; }
	if (total > 0.0f) {
		ImGui::SameLine();
		ImGui::Text("(%.1f%%)", value / total * 100.0f);
	}
}

// 説明(薄文字)
void Note(const char* text) {
	ImGui::PushTextWrapPos(0.0f);
	ImGui::TextDisabled("%s", text);
	ImGui::PopTextWrapPos();
}

} // namespace

void FloorEditor::Draw() {
	ImGui::SetNextWindowSize(ImVec2(660.0f, 700.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("フロアエディタ")) {
		ImGui::End();
		return;
	}

	const bool dirty = Snapshot() != savedText_;
	const std::vector<DataIssue> issues = CollectIssues();
	hasError_ = std::any_of(issues.begin(), issues.end(), [](const DataIssue& i) { return i.isError; });

	ImGui::TextDisabled("%s", path_.c_str());
	if (dirty) {
		ImGui::SameLine();
		ImGui::TextColored(kWarnColor, "(未保存)");
	}

	ImGui::BeginDisabled(hasError_);
	if (ImGui::Button("保存")) { Save(); }
	ImGui::EndDisabled();
	if (hasError_ && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip("エラーを直すと保存できます");
	}
	ImGui::SameLine();
	if (ImGui::Button("再読込")) {
		if (dirty) { openReloadPopup_ = true; } else { Load(); }
	}
	ImGui::SameLine();
	if (ImGui::Button("既定値に戻す")) { config_ = FloorConfig{}; }
	if (onRegenerate_) {
		ImGui::SameLine();
		if (ImGui::Button("フロアを作り直す")) { openRegeneratePopup_ = true; }
	}

	if (!status_.empty()) { ImGui::TextDisabled("%s", status_.c_str()); }
	for (const auto& issue : issues) {
		ImGui::TextColored(issue.isError ? kErrorColor : kWarnColor, "%s", issue.message.c_str());
	}
	ImGui::Separator();

	if (ImGui::BeginTabBar("floorTabs")) {
		if (ImGui::BeginTabItem("固定設定")) {
			DrawFixedTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("確率設定")) {
			DrawRateTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	DrawPopups();
	ImGui::End();
}

void FloorEditor::DrawFixedTab() {
	// ゴールまでの移動回数
	ImGui::SeparatorText("ゴール");
	ImGui::SetNextItemWidth(140.0f);
	ImGui::InputInt("ゴールまでの移動回数", &config_.goalDepth);
	config_.goalDepth = std::clamp(config_.goalDepth, 1, kMaxGoalDepth);
	ImGui::TextDisabled("部屋は、入る直前に次の分だけ作られます");

	// 階層ごとの固定
	ImGui::SeparatorText("固定出現階層");
	Note("確定で指定した種類の部屋になります。種類は部屋の中身(敵・報酬の抽選)にも使われます。");

	int removeIndex = -1;
	if (ImGui::BeginTable("fixed", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("階層", ImGuiTableColumnFlags_WidthFixed, 110.0f);
		ImGui::TableSetupColumn("種類", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.0f);
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < config_.fixedRooms.size(); ++i) {
			FixedRoom& room = config_.fixedRooms[i];
			ImGui::PushID(static_cast<int>(i));
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputInt("##depth", &room.depth, 0, 0);

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##type", RoomTypeLabel(room.type))) {
				for (RoomType type : kFixedTypes) {
					const bool selected = (type == room.type);
					if (ImGui::Selectable(RoomTypeLabel(type), selected)) { room.type = type; }
					if (selected) { ImGui::SetItemDefaultFocus(); }
				}
				ImGui::EndCombo();
			}

			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("削除")) { removeIndex = static_cast<int>(i); }
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	if (removeIndex >= 0) { config_.fixedRooms.erase(config_.fixedRooms.begin() + removeIndex); }

	if (ImGui::Button("追加")) { AddFixedRoom(); }

	// 階層ごとの内訳
	if (ImGui::TreeNodeEx("階層ごとの内訳", ImGuiTreeNodeFlags_DefaultOpen)) {
		const int last = (std::min)(config_.goalDepth, kMaxGoalDepth);
		for (int depth = 1; depth <= last; ++depth) {
			if (depth >= config_.goalDepth) {
				ImGui::TextColored(kFixedColor, "%2d  %s", depth, RoomTypeLabel(RoomType::Goal));
			} else if (const auto fixed = config_.FixedTypeAt(depth)) {
				ImGui::TextColored(kFixedColor, "%2d  %s(固定)", depth, RoomTypeLabel(*fixed));
			} else {
				ImGui::Text("%2d  ランダム", depth);
			}
		}
		ImGui::TreePop();
	}
}

void FloorEditor::DrawRateTab() {
	// 部屋タイプの確率
	ImGui::SeparatorText("部屋タイプの確率");
	Note("ランダムに選ばれる部屋の重みです。固定した階層とゴールには使われません。");

	float total = 0.0f;
	for (const auto& rate : config_.roomRates) { total += (std::max)(rate.weight, 0.0f); }

	if (ImGui::BeginTable("rates", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("種類");
		ImGui::TableSetupColumn("重み");
		ImGui::TableSetupColumn("連続の補正(倍)");
		ImGui::TableSetupColumn("出現開始階層");
		ImGui::TableSetupColumn("基本確率");
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < config_.roomRates.size(); ++i) {
			RoomTypeRate& rate = config_.roomRates[i];
			ImGui::PushID(static_cast<int>(i));
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(RoomTypeLabel(rate.type));

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##weight", &rate.weight, 0.0f, 0.0f, "%.1f");
			rate.weight = (std::max)(rate.weight, 0.0f);

			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##repeat", &rate.repeatMultiplier, 0.0f, 0.0f, "%.2f");
			rate.repeatMultiplier = (std::max)(rate.repeatMultiplier, 0.0f);

			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputInt("##minDepth", &rate.minDepth, 0, 0);
			rate.minDepth = (std::max)(rate.minDepth, 1);

			ImGui::TableSetColumnIndex(4);
			if (total > 0.0f) { ImGui::Text("%.1f%%", (std::max)(rate.weight, 0.0f) / total * 100.0f); }
			else { ImGui::TextDisabled("-"); }

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	Note("連続の補正: 直前の部屋が同じ種類のとき、その種類の重みに掛ける倍率です(1で補正なし、0で連続出現なし)。");
	Note("出現開始階層: この階層より前には選ばれません。");

	// 確率プレビュー(ゲームの抽選と同じ計算)
	if (ImGui::TreeNodeEx("確率プレビュー", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SetNextItemWidth(120.0f);
		ImGui::InputInt("階層", &previewDepth_);
		previewDepth_ = (std::max)(previewDepth_, 1);

		ImGui::SetNextItemWidth(160.0f);
		if (ImGui::BeginCombo("直前の部屋", RoomTypeLabel(previewParent_))) {
			for (RoomType type : kParentTypes) {
				const bool selected = (type == previewParent_);
				if (ImGui::Selectable(RoomTypeLabel(type), selected)) { previewParent_ = type; }
				if (selected) { ImGui::SetItemDefaultFocus(); }
			}
			ImGui::EndCombo();
		}

		if (previewDepth_ >= config_.goalDepth) {
			ImGui::TextColored(kFixedColor, "%s(移動回数で決まります)", RoomTypeLabel(RoomType::Goal));
		} else if (const auto fixed = config_.FixedTypeAt(previewDepth_)) {
			ImGui::TextColored(kFixedColor, "%s(固定)", RoomTypeLabel(*fixed));
		} else {
			const std::vector<float> p = config_.RoomTypeProbabilities(previewDepth_, previewParent_);
			if (std::all_of(p.begin(), p.end(), [](float v) { return v <= 0.0f; })) {
				ImGui::TextColored(kWarnColor, "抽選できる部屋がないため、%sになります", RoomTypeLabel(RoomType::Combat));
			} else {
				for (size_t i = 0; i < p.size(); ++i) {
					ImGui::Text("%s  %.1f%%", RoomTypeLabel(config_.roomRates[i].type), p[i] * 100.0f);
				}
			}
		}
		ImGui::TreePop();
	}

	// 報酬の確率
	ImGui::SeparatorText("報酬の確率");
	Note("部屋タイプごとの、クリア後チェストの中身です。数値は重みで、右の%は合計に対する割合です。");
	DrawReward("戦闘", RoomType::Combat);
	DrawReward("強敵", RoomType::Elite);
	DrawReward("ボス", RoomType::Boss);
}

void FloorEditor::DrawReward(const char* title, RoomType type) {
	if (!ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen)) { return; }
	ImGui::PushID(static_cast<int>(type));

	RewardRate& r = config_.RewardOf(type);

	ImGui::TextDisabled("報酬の種類");
	const float kindTotal = (std::max)(r.money, 0.0f) + (std::max)(r.weapon, 0.0f) + (std::max)(r.passive, 0.0f);
	WeightRow("お金", r.money, kindTotal);
	WeightRow("武器", r.weapon, kindTotal);
	WeightRow("パッシブ", r.passive, kindTotal);

	ImGui::TextDisabled("武器のレアリティ");
	ImGui::BeginDisabled(r.weapon <= 0.0f);
	const float rarityTotal = (std::max)(r.common, 0.0f) + (std::max)(r.rare, 0.0f) + (std::max)(r.epic, 0.0f) + (std::max)(r.legendary, 0.0f);
	WeightRow("Common", r.common, rarityTotal);
	WeightRow("Rare", r.rare, rarityTotal);
	WeightRow("Epic", r.epic, rarityTotal);
	WeightRow("Legendary", r.legendary, rarityTotal);
	ImGui::EndDisabled();
	if (r.weapon <= 0.0f) { ImGui::TextDisabled("(武器の重みが0のため使われません)"); }

	ImGui::BeginDisabled(r.money <= 0.0f);
	ImGui::SetNextItemWidth(110.0f);
	ImGui::InputInt("お金の額", &r.moneyAmount, 0, 0);
	r.moneyAmount = (std::max)(r.moneyAmount, 0);
	ImGui::EndDisabled();

	ImGui::PopID();
}

void FloorEditor::DrawPopups() {
	if (openReloadPopup_) { ImGui::OpenPopup("再読込の確認"); openReloadPopup_ = false; }
	if (openRegeneratePopup_) { ImGui::OpenPopup("フロアを作り直す"); openRegeneratePopup_ = false; }

	if (ImGui::BeginPopupModal("再読込の確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("保存していない変更は破棄されます。");
		if (ImGui::Button("再読込する")) {
			Load();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("フロアを作り直す", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		const bool dirty = Snapshot() != savedText_;
		ImGui::TextUnformatted("現在の進行を捨てて、最初の部屋から新しいフロアを作ります。");
		if (dirty) { ImGui::TextColored(kWarnColor, "未保存の変更があります。保存してから作り直します。"); }

		ImGui::BeginDisabled(dirty && hasError_);
		if (ImGui::Button(dirty ? "保存して作り直す" : "作り直す")) {
			if (!dirty || Save()) {
				if (onRegenerate_) { onRegenerate_(); }
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

#else

void FloorEditor::Draw() {}

#endif
