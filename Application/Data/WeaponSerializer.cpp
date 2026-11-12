#include "Data/WeaponSerializer.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>

namespace {

using json = nlohmann::json;
using ojson = nlohmann::ordered_json;

// 小数点以下での誤差を修正
ojson F(float v) {
	if (!std::isfinite(v)) { return 0.0; }
	char buffer[64];
	const auto result = std::to_chars(buffer, buffer + sizeof(buffer), v);
	std::string text(buffer, result.ptr);
	// 整数に見える値は.0
	if (text.find_first_of(".eE") == std::string::npos) { text += ".0"; }
	return ojson::parse(text);
}

// キーが無い・型が違う場合は既定値を返す
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

// オブジェクトならそのポインタ
const json* Obj(const json& j, const char* key) {
	const auto it = j.find(key);
	return (it != j.end() && it->is_object()) ? &(*it) : nullptr;
}

} // namespace

namespace WeaponSerializer {

bool IsEnemyWeapon(const std::string& name) {
	constexpr std::string_view suffix = "_Enemy";
	return name.size() > suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
}

WeaponData FromJson(const std::string& name, const json& j) {
	WeaponData w{};
	w.name = name;

	// 既定値
	w.rarity = Rarity::Common;
	w.stats = { 0.5f, 0.5f, 1.0f, 1.0f };
	w.bullet.damage = 1.0f;
	w.bullet.radius = 0.3f;
	w.bullet.speed = 10.0f;
	w.bullet.lifeTime = 60;
	w.bullet.knockback = 0.2f;
	w.bullet.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (!j.is_object()) { return w; }

	w.modelName = Get<std::string>(j, "modelName", "");
	w.rarity = Get<Rarity>(j, "rarity", Rarity::Common);

	w.stats.weight = Get(j, "weight", w.stats.weight);
	w.stats.shootCoolTime = Get(j, "shootCoolTime", w.stats.shootCoolTime);
	w.stats.maxCharge = Get(j, "maxCharge", w.stats.maxCharge);
	w.stats.chargeTime = Get(j, "chargeTime", w.stats.chargeTime);

	// 弾
	if (const json* bullet = Obj(j, "bullet")) {
		w.bullet.damage = Get(*bullet, "damage", w.bullet.damage);
		w.bullet.radius = Get(*bullet, "radius", w.bullet.radius);
		w.bullet.speed = Get(*bullet, "speed", w.bullet.speed);
		w.bullet.lifeTime = Get(*bullet, "lifeTime", w.bullet.lifeTime);
		w.bullet.knockback = Get(*bullet, "knockback", w.bullet.knockback);

		const auto color = bullet->find("color");
		if (color != bullet->end() && color->is_array() && color->size() >= 3) {
			try {
				w.bullet.color.x = (*color)[0].get<float>();
				w.bullet.color.y = (*color)[1].get<float>();
				w.bullet.color.z = (*color)[2].get<float>();
				w.bullet.color.w = color->size() >= 4 ? (*color)[3].get<float>() : 1.0f;
			} catch (...) {}
		}

		// 弾の性質
		if (const json* traits = Obj(*bullet, "traits")) {
			auto& t = w.bullet.traits;

			if (const json* move = Obj(*traits, "move")) {
				if (const json* v = Obj(*move, "orbit")) {
					Orbit o{};
					o.radius = Get(*v, "radius", 3.0f);
					o.speed = Get(*v, "speed", 2.0f);
					t.move.orbit = o;
				}
				if (const json* v = Obj(*move, "wave")) {
					Wave o{};
					o.amplitude = Get(*v, "amplitude", 1.0f);
					o.speed = Get(*v, "speed", 1.0f);
					o.maxAngle = Get(*v, "maxAngle", 30.0f);
					t.move.wave = o;
				}
				if (const json* v = Obj(*move, "accel")) {
					Accel o{};
					o.rate = Get(*v, "rate", 1.02f);
					t.move.accel = o;
				}
			}

			if (const json* any = Obj(*traits, "onHitAnything")) {
				if (const json* v = Obj(*any, "explode")) {
					Explode o{};
					o.radius = Get(*v, "radius", 3.0f);
					// 未指定なら弾のダメージをそのまま使う
					o.damage = Get(*v, "damage", w.bullet.damage);
					t.onHitAnything.explode = o;
				}
			}

			if (const json* wall = Obj(*traits, "onHitWall")) {
				if (const json* v = Obj(*wall, "ricochet")) {
					Ricochet o{};
					o.count = Get(*v, "count", 1);
					o.onHitEnemy = Get(*v, "onHitEnemy", false);
					o.toEnemy = Get(*v, "toEnemy", false);
					t.onHitWall.ricochet = o;
				}
			}

			if (const json* enemy = Obj(*traits, "onHitEnemy")) {
				if (const json* v = Obj(*enemy, "piercing")) {
					Piercing o{};
					o.count = Get(*v, "count", 1);
					t.onHitEnemy.piercing = o;
				}
				if (const json* v = Obj(*enemy, "slow")) {
					Slow o{};
					o.rate = Get(*v, "rate", 50.0f);
					o.time = Get(*v, "time", 2.0f);
					t.onHitEnemy.slow = o;
				}
				if (const json* v = Obj(*enemy, "flame")) {
					Flame o{};
					o.stack = Get(*v, "stack", 1.0f);
					o.interval = Get(*v, "interval", 1.0f);
					o.damage = Get(*v, "damage", 1.0f);
					t.onHitEnemy.flame = o;
				}
				if (const json* v = Obj(*enemy, "freeze")) {
					Freeze o{};
					o.stack = Get(*v, "stack", 1.0f);
					t.onHitEnemy.freeze = o;
				}
				if (const json* v = Obj(*enemy, "volt")) {
					Volt o{};
					o.range = Get(*v, "range", 3.0f);
					o.damage = Get(*v, "damage", 0.5f);
					t.onHitEnemy.volt = o;
				}
			}
		}
	}

	// 武器の性質
	if (const json* traits = Obj(j, "traits")) {
		if (const json* v = Obj(*traits, "multiShot")) {
			MultiShotParam o{};
			o.count = static_cast<int8_t>(std::clamp(Get(*v, "count", 3), 1, 127));
			o.maxAngle = Get(*v, "maxAngle", 30.0f);
			o.random = Get(*v, "random", false);
			w.traits.multiShot = o;
		}
		if (const json* v = Obj(*traits, "charge")) {
			ChargeParam o{};
			o.time = Get(*v, "time", 1.0f);
			o.damage = Get(*v, "damage", 5.0f);
			w.traits.charge = o;
		}
		if (const json* v = Obj(*traits, "burst")) {
			BurstParam o{};
			o.count = Get(*v, "count", 3);
			o.interval = Get(*v, "interval", 0.1f);
			w.traits.burst = o;
		}
	}

	return w;
}

ojson ToJson(const WeaponData& w) {
	ojson j = ojson::object();

	j["modelName"] = w.modelName;
	j["weight"] = F(w.stats.weight);
	j["shootCoolTime"] = F(w.stats.shootCoolTime);
	j["maxCharge"] = F(w.stats.maxCharge);
	j["chargeTime"] = F(w.stats.chargeTime);

	// 弾
	ojson bullet = ojson::object();
	bullet["damage"] = F(w.bullet.damage);
	bullet["radius"] = F(w.bullet.radius);
	bullet["speed"] = F(w.bullet.speed);
	bullet["lifeTime"] = w.bullet.lifeTime;
	bullet["knockback"] = F(w.bullet.knockback);
	bullet["color"] = ojson::array({ F(w.bullet.color.x), F(w.bullet.color.y), F(w.bullet.color.z), F(w.bullet.color.w) });

	// 弾の性質
	const auto& t = w.bullet.traits;
	ojson traits = ojson::object();

	ojson move = ojson::object();
	if (t.move.orbit) {
		move["orbit"] = { { "radius", F(t.move.orbit->radius) }, { "speed", F(t.move.orbit->speed) } };
	}
	if (t.move.wave) {
		move["wave"] = { { "amplitude", F(t.move.wave->amplitude) }, { "speed", F(t.move.wave->speed) }, { "maxAngle", F(t.move.wave->maxAngle) } };
	}
	if (t.move.accel) {
		move["accel"] = { { "rate", F(t.move.accel->rate) } };
	}
	if (!move.empty()) { traits["move"] = move; }

	if (t.onHitAnything.explode) {
		traits["onHitAnything"] = { { "explode", { { "damage", F(t.onHitAnything.explode->damage) }, { "radius", F(t.onHitAnything.explode->radius) } } } };
	}

	if (t.onHitWall.ricochet) {
		const auto& r = *t.onHitWall.ricochet;
		traits["onHitWall"] = { { "ricochet", { { "count", r.count }, { "onHitEnemy", r.onHitEnemy }, { "toEnemy", r.toEnemy } } } };
	}

	ojson onHitEnemy = ojson::object();
	if (t.onHitEnemy.piercing) {
		onHitEnemy["piercing"] = { { "count", t.onHitEnemy.piercing->count } };
	}
	if (t.onHitEnemy.slow) {
		onHitEnemy["slow"] = { { "rate", F(t.onHitEnemy.slow->rate) }, { "time", F(t.onHitEnemy.slow->time) } };
	}
	if (t.onHitEnemy.flame) {
		const auto& f = *t.onHitEnemy.flame;
		onHitEnemy["flame"] = { { "stack", F(f.stack) }, { "interval", F(f.interval) }, { "damage", F(f.damage) } };
	}
	if (t.onHitEnemy.freeze) {
		onHitEnemy["freeze"] = { { "stack", F(t.onHitEnemy.freeze->stack) } };
	}
	if (t.onHitEnemy.volt) {
		onHitEnemy["volt"] = { { "range", F(t.onHitEnemy.volt->range) }, { "damage", F(t.onHitEnemy.volt->damage) } };
	}
	if (!onHitEnemy.empty()) { traits["onHitEnemy"] = onHitEnemy; }

	if (!traits.empty()) { bullet["traits"] = traits; }
	j["bullet"] = bullet;

	// 武器の性質
	ojson weaponTraits = ojson::object();
	if (w.traits.multiShot) {
		const auto& m = *w.traits.multiShot;
		weaponTraits["multiShot"] = { { "count", int(m.count) }, { "maxAngle", F(m.maxAngle) }, { "random", m.random } };
	}
	if (w.traits.charge) {
		weaponTraits["charge"] = { { "time", F(w.traits.charge->time) }, { "damage", F(w.traits.charge->damage) } };
	}
	if (w.traits.burst) {
		weaponTraits["burst"] = { { "count", w.traits.burst->count }, { "interval", F(w.traits.burst->interval) } };
	}
	if (!weaponTraits.empty()) { j["traits"] = weaponTraits; }

	j["rarity"] = w.rarity;
	return j;
}

float EstimateRange(const WeaponData& w) {
	const auto& b = w.bullet;
	if (b.traits.move.orbit) { return b.traits.move.orbit->radius; }

	// Bullet::Move と同じく、加速は毎フレーム速度に掛け、1/60秒分進む
	const float rate = (b.traits.move.accel && !b.traits.move.wave) ? b.traits.move.accel->rate : 1.0f;
	float velocity = b.speed;
	float position = 0.0f;
	for (int i = 0; i < b.lifeTime; ++i) {
		velocity *= rate;
		position += velocity / 60.0f;
		if (position >= kMaxRange) { return kMaxRange; }
	}
	return position;
}

std::vector<DataIssue> Validate(const WeaponData& w) {
	std::vector<DataIssue> issues;
	auto error = [&](std::string m) { issues.push_back({ true, std::move(m) }); };
	auto warn = [&](std::string m) { issues.push_back({ false, std::move(m) }); };

	const bool isEnemy = IsEnemyWeapon(w.name);
	const auto& b = w.bullet;
	const auto& t = b.traits;

	if (w.modelName.empty()) { error("モデルが選ばれていません"); }
	if (b.speed <= 0.0f) { error("弾速は0より大きくしてください"); }
	if (b.lifeTime <= 0) { error("弾の寿命は1フレーム以上にしてください"); }
	if (b.radius <= 0.0f) { error("弾の大きさは0より大きくしてください"); }
	if (b.damage < 0.0f) { warn("ダメージが負の値です(回復になります)"); }
	if (w.stats.shootCoolTime <= 0.0f) { warn("クールタイムが0以下です(毎フレーム発射します)"); }

	if (!isEnemy) {
		if (w.stats.chargeTime <= 0.0f) { error("プレイヤー武器の1発分のチャージ時間は0より大きくしてください"); }
		if (w.stats.maxCharge < 1.0f) { warn("最大チャージが1未満のため、プレイヤーは撃てません"); }
	}

	// 武器の性質
	if (w.traits.multiShot) {
		const auto& m = *w.traits.multiShot;
		if (m.count < 2) { warn("同時発射数が1以下です(単発と同じ)"); }
		if (m.count >= 2 && m.maxAngle <= 0.0f) { warn("拡散角度が0以下のため、全弾が同じ向きに飛びます"); }
	}
	if (w.traits.charge) {
		if (w.traits.charge->time <= 0.0f) { error("溜め時間は0より大きくしてください"); }
	}
	if (w.traits.burst) {
		const auto& bu = *w.traits.burst;
		if (bu.count < 2) { warn("バースト回数が1以下です(単発と同じ)"); }
		if (bu.interval <= 0.0f) { warn("バーストの間隔が0以下です"); }
		const float total = float((std::max)(bu.count - 1, 0)) * bu.interval;
		if (w.stats.shootCoolTime < total) {
			warn(std::format("クールタイム({:.2f}秒)がバーストの所要時間({:.2f}秒)より短く、次の射撃と重なります", w.stats.shootCoolTime, total));
		}
	}

	// 弾の性質
	const bool hasOrbit = t.move.orbit.has_value();
	const bool hasWave = t.move.wave.has_value();
	if (hasOrbit && (hasWave || t.move.accel || t.onHitWall.ricochet)) {
		warn("軌道弾は、蛇行・加速・跳弾が効きません");
	} else if (hasWave && (t.move.accel || t.onHitWall.ricochet)) {
		warn("蛇行弾は、加速・跳弾が効きません");
	}
	if (hasOrbit && t.move.orbit->radius <= 0.0f) { warn("軌道の半径が0以下です"); }
	if (t.move.accel && t.move.accel->rate <= 0.0f) { error("加速率は0より大きくしてください(毎フレーム速度に掛けます)"); }
	if (t.onHitAnything.explode) {
		if (t.onHitAnything.explode->radius <= 0.0f) { error("爆発の半径は0より大きくしてください"); }
		if (t.onHitAnything.explode->damage < 0.0f) { error("爆発のダメージは0以上にしてください"); }
	}
	if (t.onHitWall.ricochet && t.onHitWall.ricochet->count < 1) { warn("跳弾の回数が0以下です"); }
	if (t.onHitEnemy.piercing && t.onHitEnemy.piercing->count < 0) { error("貫通回数は0以上にしてください"); }

	return issues;
}

} // namespace WeaponSerializer
