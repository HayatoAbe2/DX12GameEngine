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

	/*auto matData = shadowModel->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel->GetMaterial(0)->SetData(matData);
	*/
	WeaponData data;

	if (index == -1) { index = ctx.RandomInt(0, int(WEAPON::Burstgun)); }
	MaterialData matData = {};
	switch (index) {
	case static_cast<int>(WEAPON::Pistol):
	{
		data = weaponDataMap["Pistol"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Pistol>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::AssaultRifle):
	{
		data = weaponDataMap["AssaultRifle"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<AssaultRifle>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Shotgun):
	{
		data = weaponDataMap["Shotgun"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Shotgun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::FireBall):
	{
		data = weaponDataMap["Spellbook"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<FireBall>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Wavegun):
	{
		data = weaponDataMap["Wavegun"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Wavegun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::OrbitStaff):
	{
		data = weaponDataMap["OrbitStaff"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<OrbitStaff>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::ChargeGun):
	{
		data = weaponDataMap["ChargeGun"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<ChargeGun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::AccelGun):
	{
		data = weaponDataMap["AccelGun"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<AccelGun>(data, std::move(model), std::move(shadowModel));
	}
	case static_cast<int>(WEAPON::Sniper):
	{
		data = weaponDataMap["Sniper"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Sniper>(data, std::move(model), std::move(shadowModel));
	}
	default:
	{
		data = weaponDataMap["Burstgun"];
		auto model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		auto shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
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
		WeaponData wd{};

		wd.name = name;
		wd.modelName = data["modelName"];

		wd.stats.damage = data["damage"];
		wd.stats.weight = data["weight"];

		wd.stats.bulletSize = data["bulletSize"];
		wd.stats.bulletSpeed = data["bulletSpeed"];

		wd.stats.shootCoolTime = data["shootCoolTime"];
		wd.stats.bulletLifeTime = data["bulletLifeTime"];

		wd.stats.knockback = data["knockback"];

		auto color = data["bulletColor"];
		wd.bulletColor.x = color[0];
		wd.bulletColor.y = color[1];
		wd.bulletColor.z = color[2];
		wd.bulletColor.w = color[3];

		// enum変換機能
		wd.rarity = data["rarity"].get<Rarity>();

		weaponDataMap[name] = wd;
	}
}