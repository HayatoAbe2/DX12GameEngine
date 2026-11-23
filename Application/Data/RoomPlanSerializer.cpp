#include "Data/RoomPlanSerializer.h"
#include <algorithm>

namespace {

using json = nlohmann::json;
using ojson = nlohmann::ordered_json;

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

// enum と文字列の対応表。先頭の項目が、見つからないときの既定値
template <typename E>
struct Entry {
	E value;
	const char* name;
};

constexpr Entry<RoomType> kRoomTypes[] = {
	{ RoomType::Start, "Start" }, { RoomType::Combat, "Combat" }, { RoomType::Elite, "Elite" },
	{ RoomType::Boss, "Boss" }, { RoomType::Shop, "Shop" }, { RoomType::Event, "Event" }, { RoomType::Goal, "Goal" } };

constexpr Entry<RewardType> kRewardTypes[] = {
	{ RewardType::None, "None" }, { RewardType::Money, "Money" },
	{ RewardType::Weapon, "Weapon" }, { RewardType::Passive, "Passive" } };

constexpr Entry<Rarity> kRarities[] = {
	{ Rarity::None, "None" }, { Rarity::Common, "Common" }, { Rarity::Rare, "Rare" },
	{ Rarity::Epic, "Epic" }, { Rarity::Legendary, "Legendary" } };

template <typename E, size_t N>
const char* NameOf(const Entry<E> (&table)[N], E value) {
	for (const auto& e : table) {
		if (e.value == value) { return e.name; }
	}
	return table[0].name;
}

// 見つかったときだけ out を書き換える
template <typename E, size_t N>
bool Parse(const Entry<E> (&table)[N], const std::string& text, E& out) {
	for (const auto& e : table) {
		if (text == e.name) {
			out = e.value;
			return true;
		}
	}
	return false;
}

// この部屋かその先に、予定のある部屋があるか
bool HasPlan(const Room& room) {
	if (room.plan.planned) { return true; }
	for (const auto& child : room.nextRooms) {
		if (HasPlan(child)) { return true; }
	}
	return false;
}

ojson NodeToJson(const Room& room) {
	ojson j = ojson::object();
	j["type"] = NameOf(kRoomTypes, room.type);
	if (room.plan.planned) {
		j["plan"] = RoomPlanSerializer::ToJson(room.plan);
	}

	// 先に予定がある場合だけ next を出す。兄弟は添字がずれないよう、予定がなくても type だけ出す
	bool any = false;
	for (const auto& child : room.nextRooms) {
		if (HasPlan(child)) { any = true; break; }
	}
	if (any) {
		ojson next = ojson::array();
		for (const auto& child : room.nextRooms) {
			next.push_back(NodeToJson(child));
		}
		j["next"] = std::move(next);
	}
	return j;
}

void ApplyNode(Room& room, int depth, int maxDepth, bool canGrow, const json& j, std::vector<DataIssue>* issues, const std::string& path) {
	if (!j.is_object()) { return; }

	const auto type = j.find("type");
	if (type != j.end() && type->is_string()) {
		const std::string text = type->get<std::string>();
		RoomType t = room.type;
		if (Parse(kRoomTypes, text, t)) {
			room.type = t;
		} else if (issues) {
			issues->push_back({ false, path + ": 不明な部屋タイプ「" + text + "」(変更しません)" });
		}
	}

	const auto plan = j.find("plan");
	if (plan != j.end()) {
		room.plan = RoomPlanSerializer::FromJson(*plan, issues, path);
	}

	const auto next = j.find("next");
	if (next != j.end() && next->is_array()) {
		// まだ次の部屋がなければ、ファイルの分を作る(ゴールの先と、4つ目以降は作らない)
		if (canGrow && room.nextRooms.empty() && depth < maxDepth) {
			const size_t count = std::min<size_t>(next->size(), 3);
			for (size_t i = 0; i < count; ++i) {
				Room created;
				created.type = RoomType::Combat; // 型は直後にファイルの値で上書きされる
				room.nextRooms.push_back(std::move(created));
			}
		}

		const size_t n = min(next->size(), room.nextRooms.size());
		for (size_t i = 0; i < n; ++i) {
			// 作ったばかりの部屋の先は、さらにファイルから作れる
			ApplyNode(room.nextRooms[i], depth + 1, maxDepth, canGrow, (*next)[i], issues, path + "/" + std::to_string(i));
		}
		if (issues && next->size() > room.nextRooms.size()) {
			issues->push_back({ false, path + ": next が " + std::to_string(room.nextRooms.size()) + " 個を超えています(超えた分は無視)" });
		}
	}
}

void ValidateNode(const Room& room, const std::vector<std::string>& enemyNames, const std::string& path, std::vector<DataIssue>& out) {
	if (room.plan.planned) {
		for (auto issue : RoomPlanSerializer::Validate(room.plan, room.type, enemyNames)) {
			issue.message = path + ": " + issue.message;
			out.push_back(std::move(issue));
		}
	}
	for (size_t i = 0; i < room.nextRooms.size(); ++i) {
		if (HasPlan(room.nextRooms[i])) {
			ValidateNode(room.nextRooms[i], enemyNames, path + "/" + std::to_string(i), out);
		}
	}
}

} // namespace

namespace RoomPlanSerializer {

RoomPlan FromJson(const json& j, std::vector<DataIssue>* issues, const std::string& where) {
	RoomPlan plan{};
	plan.planned = true; // ファイルに書かれた部屋は確定済みとして扱う(再抽選しない)
	if (!j.is_object()) { return plan; }

	plan.enemies = Get<std::vector<std::string>>(j, "enemies", {});

	const auto reward = j.find("reward");
	if (reward != j.end() && reward->is_object()) {
		const std::string type = Get<std::string>(*reward, "type", "None");
		if (!Parse(kRewardTypes, type, plan.reward.type) && issues) {
			issues->push_back({ false, where + ": 不明な報酬タイプ「" + type + "」(None扱い)" });
		}

		const std::string rarity = Get<std::string>(*reward, "rarity", "Common");
		plan.reward.rarity = Rarity::Common;
		if (!Parse(kRarities, rarity, plan.reward.rarity) && issues) {
			issues->push_back({ false, where + ": 不明なレアリティ「" + rarity + "」(Common扱い)" });
		}

		plan.reward.amount = Get(*reward, "amount", 0);
	}
	return plan;
}

ojson ToJson(const RoomPlan& plan) {
	ojson j = ojson::object();
	j["enemies"] = plan.enemies;
	j["reward"] = {
		{ "type", NameOf(kRewardTypes, plan.reward.type) },
		{ "rarity", NameOf(kRarities, plan.reward.rarity) },
		{ "amount", plan.reward.amount } };
	return j;
}

std::vector<DataIssue> Validate(const RoomPlan& plan, RoomType type, const std::vector<std::string>& enemyNames) {
	std::vector<DataIssue> issues;
	auto error = [&](std::string m) { issues.push_back({ true, std::move(m) }); };
	auto warn = [&](std::string m) { issues.push_back({ false, std::move(m) }); };

	const bool fights = type == RoomType::Combat || type == RoomType::Elite || type == RoomType::Boss;

	if (fights && plan.enemies.empty()) {
		warn("戦闘部屋なのに敵がいません(すぐにクリア扱いになり、チェストも出ません)");
	}
	if (!fights && !plan.enemies.empty()) {
		warn("戦闘のない部屋に敵が指定されています");
	}
	for (const auto& name : plan.enemies) {
		if (std::find(enemyNames.begin(), enemyNames.end(), name) == enemyNames.end()) {
			error("登録されていない敵「" + name + "」です(出現しません)");
		}
	}

	const RewardPlan& r = plan.reward;
	if (r.amount < 0) { error("報酬の額は0以上にしてください"); }
	if (r.type == RewardType::Money && r.amount == 0) { warn("お金の報酬なのに額が0です"); }
	if (r.type == RewardType::Weapon && r.rarity == Rarity::None) { warn("武器の報酬のレアリティが未指定です"); }
	if (!fights && r.type != RewardType::None) { warn("戦闘のない部屋の報酬は使われません"); }

	return issues;
}

ojson TreeToJson(const Room& root) {
	return NodeToJson(root);
}

void ApplyTree(Room& root, const json& j, int maxDepth, std::vector<DataIssue>* issues) {
	ApplyNode(root, 0, maxDepth, true, j, issues, "root");
}

std::vector<DataIssue> ValidateTree(const Room& root, const std::vector<std::string>& enemyNames) {
	std::vector<DataIssue> issues;
	ValidateNode(root, enemyNames, "root", issues);
	return issues;
}

} // namespace RoomPlanSerializer
