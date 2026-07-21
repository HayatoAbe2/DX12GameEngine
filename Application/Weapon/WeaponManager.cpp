#include "WeaponManager.h"
#include "Weapon.h"
#include <Externals/nlohmann/json.hpp>

void WeaponManager::Initialize() {
	// json読み込み
	LoadJson("Resources/Weapons/Data/Pistol.json");
	LoadJson("Resources/Weapons/Data/AssaultRifle.json");
	LoadJson("Resources/Weapons/Data/Shotgun.json");
	LoadJson("Resources/Weapons/Data/Spellbook.json");
	LoadJson("Resources/Weapons/Data/Wavegun.json");
	LoadJson("Resources/Weapons/Data/OrbitStaff.json");
	LoadJson("Resources/Weapons/Data/Chargegun.json");
	LoadJson("Resources/Weapons/Data/Accelgun.json");
	LoadJson("Resources/Weapons/Data/Sniper.json");
	LoadJson("Resources/Weapons/Data/Burstgun.json");
}

std::unique_ptr<Weapon> WeaponManager::GetWeapon(int index, Rarity rarity) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	WeaponData data;

	// ランダム
	if (index == -1) { index = ctx.RandomInt(0, int(WEAPON::Count) - 1); }

	// 武器データ
	data = weaponDataMap[WeaponNames[index]];

	auto model = asset.LoadModel("Resources/Weapons", data.modelName);
	auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
	MaterialData matData = model->GetMaterial(0)->GetData();
	matData.color = { 0.2f,0.2f,0.2f,1 };
	model->GetMaterial(0)->SetData(matData);
	model->GetMaterial(1)->SetData(matData);

	return std::make_unique<Weapon>(data, std::move(model), std::move(shadowModel));
}

void WeaponManager::LoadJson(const std::string& path) {
	std::ifstream file(path);
	assert(file.is_open());

	nlohmann::json j;
	file >> j;

	for (auto& [name, data] : j.items()) {
		WeaponData weapon{};

		weapon.name = name;
		weapon.modelName = data["modelName"];

		// enum変換
		weapon.rarity = data["rarity"].get<Rarity>();

		weapon.stats.weight = data["weight"];
		weapon.stats.shootCoolTime = data["shootCoolTime"];
		weapon.stats.maxCharge = data["maxCharge"];
		weapon.stats.chargeTime = data["chargeTime"];

		// 弾
		auto& bullet = data["bullet"];
		weapon.bullet.damage = bullet["damage"];
		weapon.bullet.radius = bullet["radius"];
		weapon.bullet.speed = bullet["speed"];
		weapon.bullet.lifeTime = bullet["lifeTime"];
		weapon.bullet.knockback = bullet["knockback"];
		auto& color = bullet["color"];
		weapon.bullet.color.x = color[0];
		weapon.bullet.color.y = color[1];
		weapon.bullet.color.z = color[2];
		weapon.bullet.color.w = color[3];

		///
		/// 弾の性質
		///
		if (bullet.contains("traits")) {
			auto& traits = bullet["traits"];

			// 移動関連
			if (traits.contains("move")) {
				auto& move = traits["move"];

				if (move.contains("wave")) {
					Wave t;
					auto& j = move["wave"];

					t.amplitude = j["amplitude"];
					t.speed = j["speed"];
					t.maxAngle = j["maxAngle"];

					weapon.bullet.traits.move.wave = t;
				}

				if (move.contains("accel")) {
					Accel t;
					auto& j = move["accel"];

					t.rate = j["rate"];

					weapon.bullet.traits.move.accel = t;
				}

				if (move.contains("orbit")) {
					Orbit t;
					auto& j = move["orbit"];

					t.radius = j["radius"];
					t.speed = j["speed"];

					weapon.bullet.traits.move.orbit = t;
				}
			}

			// 壁ヒット時
			if (traits.contains("onHitWall")) {
				auto& wall = traits["onHitWall"];

				if (wall.contains("ricochet")) {
					Ricochet t;
					auto& j = wall["ricochet"];

					t.count = j["count"];
					t.onHitEnemy = j["onHitEnemy"];
					t.toEnemy = j["toEnemy"];

					weapon.bullet.traits.onHitWall.ricochet = t;
				}
			}

			if (traits.contains("onHitEnemy")) {
				auto& enemy = traits["onHitEnemy"];

				if (enemy.contains("piercing")) {
					Piercing t;
					auto& j = enemy["piercing"];

					t.count = j["count"];

					weapon.bullet.traits.onHitEnemy.piercing = t;
				}

				if (enemy.contains("slow")) {
					Slow t;
					auto& j = enemy["slow"];

					t.rate = j["rate"];
					t.time = j["time"];

					weapon.bullet.traits.onHitEnemy.slow = t;
				}

				if (enemy.contains("flame")) {}
				if (enemy.contains("freeze")) {}
				if (enemy.contains("volt")) {}

			}

			if (traits.contains("onHitAnything")) {
				auto& any = traits["onHitAnything"];

				if (any.contains("explode")) {
					Explode t;
					auto& j = any["explode"];

					t.radius = j["radius"];

					weapon.bullet.traits.onHitAnything.explode = t;
				}
			}
		}

		///
		/// 武器の性質
		///

		if (data.contains("traits")) {
			auto& traits = data["traits"];

			// MultiShot
			if (traits.contains("multiShot")) {
				MultiShotParam param;
				auto& j = traits["multiShot"];

				param.count = j["count"];
				param.maxAngle = j["maxAngle"];
				param.random = j["random"];

				weapon.traits.multiShot = param;
			}
			// Charge
			if (traits.contains("charge")) {
				ChargeParam param;
				auto& j = traits["charge"];

				param.time = j["time"];
				param.damage = j["damage"];

				weapon.traits.charge = param;
			}
			// Burst
			if (traits.contains("burst")) {
				BurstParam param;
				auto& j = traits["burst"];

				param.count = j["count"];
				param.interval = j["interval"];

				weapon.traits.burst = param;
			}
		}

		weaponDataMap[name] = weapon;
	}
}