#include "Data/FloorConfigSerializer.h"
#include "Data/JsonFormat.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace {

using json = nlohmann::json;
using ojson = nlohmann::ordered_json;

// float を最短の表記で数値にする(EnemySerializer と同じ)
ojson F(float v) {
	if (!std::isfinite(v)) { return 0.0; }
	char buffer[64];
	const auto result = std::to_chars(buffer, buffer + sizeof(buffer), v);
	std::string text(buffer, result.ptr);
	if (text.find_first_of(".eE") == std::string::npos) { text += ".0"; }
	return ojson::parse(text);
}

template <typename T>
T Get(const json& j, const char* key, T defaultValue) {
	const auto it = j.find(key);
	if (it == j.end()) { return defaultValue; }
	try {
		return it->get<T>();
	} catch (...) {
		return defaultValue;
	}
}

// 報酬の確率
void ReadReward(const json& j, RewardRate& r) {
	if (!j.is_object()) { return; }

	const auto kind = j.find("kind");
	if (kind != j.end() && kind->is_object()) {
		r.money = Get(*kind, "Money", r.money);
		r.weapon = Get(*kind, "Weapon", r.weapon);
		r.passive = Get(*kind, "Passive", r.passive);
	}
	const auto rarity = j.find("rarity");
	if (rarity != j.end() && rarity->is_object()) {
		r.common = Get(*rarity, "Common", r.common);
		r.rare = Get(*rarity, "Rare", r.rare);
		r.epic = Get(*rarity, "Epic", r.epic);
		r.legendary = Get(*rarity, "Legendary", r.legendary);
	}
	r.moneyAmount = Get(j, "moneyAmount", r.moneyAmount);
}

ojson WriteReward(const RewardRate& r) {
	ojson j = ojson::object();
	j["kind"] = { { "Money", F(r.money) }, { "Weapon", F(r.weapon) }, { "Passive", F(r.passive) } };
	j["rarity"] = { { "Common", F(r.common) }, { "Rare", F(r.rare) }, { "Epic", F(r.epic) }, { "Legendary", F(r.legendary) } };
	j["moneyAmount"] = r.moneyAmount;
	return j;
}

} // namespace

namespace FloorConfigSerializer {

FloorConfig FromJson(const json& j) {
	FloorConfig c{};
	if (!j.is_object()) { return c; }

	c.goalDepth = Get(j, "goalDepth", c.goalDepth);

	// 固定の部屋(ファイルに書かれていれば、既定の内容は置き換える)
	const auto fixed = j.find("fixedRooms");
	if (fixed != j.end() && fixed->is_array()) {
		c.fixedRooms.clear();
		for (const auto& item : *fixed) {
			if (!item.is_object()) { continue; }

			FixedRoom room;
			room.depth = Get(item, "depth", 1);
			room.type = RoomType::Shop;
			ParseRoomType(Get<std::string>(item, "type", ""), room.type);
			c.fixedRooms.push_back(room);
		}
	}

	// 部屋タイプの確率(書かれたタイプだけ上書きする)
	const auto rates = j.find("roomRates");
	if (rates != j.end() && rates->is_object()) {
		for (auto& rate : c.roomRates) {
			const auto it = rates->find(RoomTypeKey(rate.type));
			if (it == rates->end() || !it->is_object()) { continue; }
			rate.weight = Get(*it, "weight", rate.weight);
			rate.repeatMultiplier = Get(*it, "repeatMultiplier", rate.repeatMultiplier);
			rate.minDepth = Get(*it, "minDepth", rate.minDepth);
		}
	}

	// 報酬の確率
	const auto rewards = j.find("rewards");
	if (rewards != j.end() && rewards->is_object()) {
		for (RoomType type : { RoomType::Combat, RoomType::Elite, RoomType::Boss }) {
			const auto it = rewards->find(RoomTypeKey(type));
			if (it != rewards->end()) { ReadReward(*it, c.RewardOf(type)); }
		}
	}

	return c;
}

ojson ToJson(const FloorConfig& c) {
	ojson j = ojson::object();
	j["goalDepth"] = c.goalDepth;

	std::vector<FixedRoom> fixed = c.fixedRooms;
	std::stable_sort(fixed.begin(), fixed.end(), [](const FixedRoom& a, const FixedRoom& b) { return a.depth < b.depth; });
	j["fixedRooms"] = ojson::array();
	for (const auto& room : fixed) {
		j["fixedRooms"].push_back({ { "depth", room.depth }, { "type", RoomTypeKey(room.type) } });
	}

	j["roomRates"] = ojson::object();
	for (const auto& rate : c.roomRates) {
		j["roomRates"][RoomTypeKey(rate.type)] = {
			{ "weight", F(rate.weight) },
			{ "repeatMultiplier", F(rate.repeatMultiplier) },
			{ "minDepth", rate.minDepth } };
	}

	j["rewards"] = ojson::object();
	for (RoomType type : { RoomType::Combat, RoomType::Elite, RoomType::Boss }) {
		j["rewards"][RoomTypeKey(type)] = WriteReward(c.RewardOf(type));
	}
	return j;
}

std::vector<DataIssue> Validate(const FloorConfig& c) {
	std::vector<DataIssue> issues;
	auto error = [&](std::string m) { issues.push_back({ true, std::move(m) }); };
	auto warn = [&](std::string m) { issues.push_back({ false, std::move(m) }); };

	// ゴールまでの移動回数
	if (c.goalDepth < 1) {
		error("ゴールまでの移動回数は1以上にしてください");
	} else if (c.goalDepth > kMaxGoalDepth) {
		error("ゴールまでの移動回数は" + std::to_string(kMaxGoalDepth) + "以下にしてください");
	}

	// 固定の部屋
	for (size_t i = 0; i < c.fixedRooms.size(); ++i) {
		const FixedRoom& room = c.fixedRooms[i];
		const std::string name = std::to_string(room.depth) + "階層の固定";

		if (room.depth < 1 || room.depth >= c.goalDepth) {
			error(name + ": 階層は 1 〜 " + std::to_string(max(c.goalDepth - 1, 1)) + " にしてください(ゴールの階層は移動回数で決まります)");
		}
		if (room.type == RoomType::Start || room.type == RoomType::Goal) {
			error(name + ": スタートとゴールは固定できません");
		}
		for (size_t k = 0; k < i; ++k) {
			if (c.fixedRooms[k].depth == room.depth) {
				error(name + ": 同じ階層に複数の固定があります");
				break;
			}
		}
	}

	// 部屋タイプの確率
	bool anyWeight = false;
	for (const auto& rate : c.roomRates) {
		const std::string name = std::string(RoomTypeLabel(rate.type)) + "の確率";
		if (rate.weight < 0.0f) { error(name + ": 重みは0以上にしてください"); }
		if (rate.repeatMultiplier < 0.0f) { error(name + ": 連続の補正は0以上にしてください"); }
		if (rate.minDepth < 1) { error(name + ": 出現開始階層は1以上にしてください"); }
		if (rate.weight > 0.0f) { anyWeight = true; }
	}
	if (!anyWeight) {
		error("ランダムに選ばれる部屋の重みが全て0です");
	}

	// 報酬の確率
	for (RoomType type : { RoomType::Combat, RoomType::Elite, RoomType::Boss }) {
		const RewardRate& r = c.RewardOf(type);
		const std::string name = std::string(RoomTypeLabel(type)) + "の報酬";

		if (r.money < 0.0f || r.weapon < 0.0f || r.passive < 0.0f ||
			r.common < 0.0f || r.rare < 0.0f || r.epic < 0.0f || r.legendary < 0.0f) {
			error(name + ": 重みは0以上にしてください");
		}
		if (r.money + r.weapon + r.passive <= 0.0f) {
			error(name + ": 報酬の種類の重みが全て0です");
		}
		if (r.weapon > 0.0f && r.common + r.rare + r.epic + r.legendary <= 0.0f) {
			warn(name + ": 武器が出る設定ですが、レアリティの重みが全て0です(Commonになります)");
		}
		if (r.moneyAmount < 0) { error(name + ": お金の額は0以上にしてください"); }
		if (r.money > 0.0f && r.moneyAmount == 0) { warn(name + ": お金が出る設定ですが、額が0です"); }
	}

	return issues;
}

bool Load(const std::string& path, FloorConfig& out) {
	std::ifstream file(path);
	if (!file.is_open()) {
		out = FloorConfig{}; // ファイルが無ければ既定値
		return true;
	}

	// 例外を出さずに読む(壊れていれば discarded になる)
	const json j = json::parse(file, nullptr, false);
	if (j.is_discarded() || !j.is_object()) { return false; }

	out = FromJson(j);
	return true;
}

bool Save(const std::string& path, const FloorConfig& config) {
	std::error_code ec;
	const std::filesystem::path p(path);
	if (p.has_parent_path()) { std::filesystem::create_directories(p.parent_path(), ec); }

	std::ofstream file(p);
	if (!file.is_open()) { return false; }
	file << DumpJson(ToJson(config)) << "\n";
	return file.good();
}

} // namespace FloorConfigSerializer
