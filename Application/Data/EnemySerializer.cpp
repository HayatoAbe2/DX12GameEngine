#include "Data/EnemySerializer.h"
#include <charconv>
#include <cmath>

namespace {

using json = nlohmann::json;
using ojson = nlohmann::ordered_json;

// float を最短の表記で数値にする(WeaponSerializer と同じ)
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

} // namespace

namespace EnemySerializer {

EnemyData FromJson(const json& j) {
	EnemyData e{};
	if (!j.is_object()) { return e; }

	e.modelName = Get<std::string>(j, "modelName", "");
	e.animationIndex = Get(j, "animationIndex", -1);

	// ステータス
	e.status.hp = Get(j, "hp", e.status.hp);
	e.status.radius = Get(j, "radius", e.status.radius);
	e.status.moveSpeed = Get(j, "moveSpeed", e.status.moveSpeed);
	e.status.defaultSearchRadius = Get(j, "defaultSearchRadius", e.status.defaultSearchRadius);
	e.status.stunResist = Get(j, "stunResist", e.status.stunResist);
	e.status.canFly = Get(j, "canFly", e.status.canFly);
	e.status.attackRadius = Get(j, "attackRadius", e.status.attackRadius);

	// 特殊性能
	const auto traits = j.find("traits");
	if (traits != j.end() && traits->is_object()) {
		const auto contact = traits->find("contactDamage"); // 接触ダメージ
		if (contact != traits->end() && contact->is_object()) {
			ContactDamage c{};
			c.damage = Get(*contact, "damage", 1.0f);
			e.status.traits.contactDamage = c;
		}
	}

	e.weapons = Get<std::vector<std::string>>(j, "weapons", {});

	const auto count = j.find("spawnCount"); // 出現数
	if (count != j.end() && count->is_object()) {
		e.minSpawnCount = Get(*count, "min", 1);
		e.maxSpawnCount = Get(*count, "max", 1);
	}

	return e;
}

ojson ToJson(const EnemyData& e) {
	ojson j = ojson::object();

	j["modelName"] = e.modelName;
	j["animationIndex"] = e.animationIndex;
	j["hp"] = F(e.status.hp);
	j["radius"] = F(e.status.radius);
	j["moveSpeed"] = F(e.status.moveSpeed);
	j["defaultSearchRadius"] = F(e.status.defaultSearchRadius);
	j["stunResist"] = F(e.status.stunResist);
	j["canFly"] = e.status.canFly;
	j["attackRadius"] = F(e.status.attackRadius);

	if (e.status.traits.contactDamage) {
		j["traits"] = { { "contactDamage", { { "damage", F(e.status.traits.contactDamage->damage) } } } };
	}

	j["weapons"] = e.weapons;
	j["spawnCount"] = { { "min", e.minSpawnCount }, { "max", e.maxSpawnCount } };
	return j;
}

std::vector<DataIssue> Validate(const EnemyData& e) {
	std::vector<DataIssue> issues;
	auto error = [&](std::string m) { issues.push_back({ true, std::move(m) }); };
	auto warn = [&](std::string m) { issues.push_back({ false, std::move(m) }); };

	if (e.modelName.empty()) { error("モデルが選ばれていません"); }
	if (e.status.hp <= 0.0f) { error("HPは0より大きくしてください"); }
	if (e.status.radius <= 0.0f) { error("半径は0より大きくしてください"); }
	if (e.status.moveSpeed < 0.0f) { error("移動速度は0以上にしてください"); }
	if (e.status.stunResist < 0.0f) { warn("スタン耐性が負の値です"); }
	if (e.minSpawnCount < 1) { error("出現数の最小は1以上にしてください"); }
	if (e.minSpawnCount > e.maxSpawnCount) { error("出現数の最小が最大を超えています"); }
	if (e.status.traits.contactDamage && e.status.traits.contactDamage->damage < 0.0f) {
		error("接触ダメージは0以上にしてください");
	}

	return issues;
}

} // namespace EnemySerializer
