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

// アニメーション1つ分(キーが無ければ「なし」)
CharacterAnimation ReadAnimation(const json& parent, const char* key) {
	CharacterAnimation a{};
	const auto it = parent.find(key);
	if (it == parent.end() || !it->is_object()) { return a; }
	a.index = Get(*it, "index", -1);
	a.duration = Get(*it, "duration", -1.0f);
	if (a.duration <= 0.0f) { a.duration = -1.0f; } // 0以下は「元の長さ」に揃える
	return a;
}

// 使うアニメーションを書く
void WriteAnimation(ojson& parent, const char* key, const CharacterAnimation& a) {
	// indexが-1なら書かない
	if (a.index < 0) { return; }
	ojson j = ojson::object();
	j["index"] = a.index;

	// durationがなければ書かない
	if (a.duration > 0.0f) { j["duration"] = F(a.duration); }
	parent[key] = std::move(j);
}

} // namespace

namespace EnemySerializer {

EnemyData FromJson(const json& j) {
	EnemyData e{};
	if (!j.is_object()) { return e; }

	e.modelName = Get<std::string>(j, "modelName", "");
	
	// アニメーション
	const auto animations = j.find("animations");
	if (animations != j.end() && animations->is_object()) {
		e.animations[static_cast<int>(EnemyAnimations::Idle)] = ReadAnimation(*animations, "attack");
		e.animations[static_cast<int>(EnemyAnimations::Attack)] = ReadAnimation(*animations, "attack");
		e.animations[static_cast<int>(EnemyAnimations::Dead)] = ReadAnimation(*animations, "dead");
		e.animations[static_cast<int>(EnemyAnimations::Hit)] = ReadAnimation(*animations, "hit");
	}

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

	const auto sw = j.find("weaponSwitch"); // 武器の切り替え回数
	if (sw != j.end() && sw->is_object()) {
		e.weaponSwitch.minAttacks = Get(*sw, "min", 1);
		e.weaponSwitch.maxAttacks = Get(*sw, "max", 1);
	}

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

	ojson animations = ojson::object();
	WriteAnimation(animations, "idle", e.animations[static_cast<int>(EnemyAnimations::Idle)]);
	WriteAnimation(animations, "attack", e.animations[static_cast<int>(EnemyAnimations::Attack)]);
	WriteAnimation(animations, "dead", e.animations[static_cast<int>(EnemyAnimations::Dead)]);
	WriteAnimation(animations, "hit", e.animations[static_cast<int>(EnemyAnimations::Hit)]);
	if (!animations.empty()) { j["animations"] = std::move(animations); }
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
	if (e.weapons.size() > 1) { // 武器が1つ以下なら意味がないので書かない
		j["weaponSwitch"] = { { "min", e.weaponSwitch.minAttacks }, { "max", e.weaponSwitch.maxAttacks } };
	}
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
	const std::pair<const char*, const CharacterAnimation*> animations[] = {
		{ "待機", &e.animations[static_cast<int>(EnemyAnimations::Idle)] },
		{ "攻撃", &e.animations[static_cast<int>(EnemyAnimations::Attack)] },
		{ "死亡", &e.animations[static_cast<int>(EnemyAnimations::Dead)] },
		{ "被弾", &e.animations[static_cast<int>(EnemyAnimations::Hit)] },
	};
	for (const auto& [label, a] : animations) {
		if (a->index < -1) { error(std::string(label) + "アニメーションの番号は-1以上にしてください"); }
		if (a->index >= 0 && a->duration > 60.0f) { warn(std::string(label) + "アニメーションの再生時間が60秒を超えています"); }
	}
	if (e.minSpawnCount < 1) { error("出現数の最小は1以上にしてください"); }
	if (e.minSpawnCount > e.maxSpawnCount) { error("出現数の最小が最大を超えています"); }
	if (e.weapons.size() > 1) {
		if (e.weaponSwitch.minAttacks < 1) { error("武器を切り替える回数の最小は1以上にしてください"); }
		if (e.weaponSwitch.minAttacks > e.weaponSwitch.maxAttacks) { error("武器を切り替える回数の最小が最大を超えています"); }
	}
	if (e.status.traits.contactDamage && e.status.traits.contactDamage->damage < 0.0f) {
		error("接触ダメージは0以上にしてください");
	}

	return issues;
}

} // namespace EnemySerializer
