#include "WeaponManager.h"
#include "Weapon.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"
#include "Weapon/Weapons/OrbitStaff.h"
#include "Weapon/Weapons/AccelGun.h"
#include "Weapon/Weapons/ChargeGun.h"
#include "Weapon/Weapons/Sniper.h"
#include "Weapon/Weapons/Burstgun.h"
#include <fstream>
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

	switch (index) {
	case static_cast<int>(WEAPON::Pistol):
	{
		return std::make_unique<Pistol>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::AssaultRifle):
	{
		return std::make_unique<AssaultRifle>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Shotgun):
	{
		return std::make_unique<Shotgun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::FireBall):
	{
		return std::make_unique<FireBall>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Wavegun):
	{
		return std::make_unique<Wavegun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::OrbitStaff):
	{
		return std::make_unique<OrbitStaff>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::ChargeGun):
	{
		return std::make_unique<ChargeGun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::AccelGun):
	{
		return std::make_unique<AccelGun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Sniper):
	{
		return std::make_unique<Sniper>(data, std::move(model), std::move(shadowModel));
	}
	default:
	{
		return std::make_unique<Burstgun>(data, std::move(model), std::move(shadowModel));
	}
	}
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

		// enum変換機能
		weapon.rarity = data["rarity"].get<Rarity>();

		///
		/// 武器の性質
		///

		if (data.contains("traits")) {
			auto& traits = data["traits"];

			// MultiShot
			if (traits.contains("multiShot")) {
				MultiShotParam param;
				auto& j = traits["multiShot"];

				param.pelletCount = j["pelletCount"];
				param.maxAngle = j["maxAngle"];

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