#include "WeaponManager.h"
#include "Weapon.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"
#include <fstream>
#include <Externals/nlohmann/json.hpp>

void WeaponManager::Initialize() {
	// json読み込み
	LoadJson("Resources/Weapons/Data/Pistol.json");
	LoadJson("Resources/Weapons/Data/AssaultRifle.json");
	LoadJson("Resources/Weapons/Data/Shotgun.json");
	LoadJson("Resources/Weapons/Data/Spellbook.json");
	LoadJson("Resources/Weapons/Data/Wavegun.json");
}

std::unique_ptr<Weapon> WeaponManager::GetWeapon(int index, Rarity rarity) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	auto model = std::make_unique<Model>();
	auto shadowModel = std::make_unique<Model>();
	/*auto matData = shadowModel->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel->GetMaterial(0)->SetData(matData);
	*/
	WeaponData data;

	if (index == -1) { index = ctx.RandomInt(0, int(WEAPON::Wavegun)); }
	MaterialData matData = {};
	switch (index) {
	case static_cast<int>(WEAPON::Pistol):
		data = weaponDataMap["Pistol"];
		model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Pistol>(data, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::AssaultRifle):
		data = weaponDataMap["AssaultRifle"];
		model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<AssaultRifle>(data, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::Shotgun):
		data = weaponDataMap["Shotgun"];
		model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Shotgun>(data, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::FireBall):
		data = weaponDataMap["Spellbook"];
		model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<FireBall>(data, std::move(model), std::move(shadowModel));
		break;

	default:
		data = weaponDataMap["Wavegun"];
		model = asset.LoadModel("Resources/Weapons", data.modelName);
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", data.modelName);
		return std::make_unique<Wavegun>(data, std::move(model), std::move(shadowModel));
		break;
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

		wd.damage = data["damage"];
		wd.weight = data["weight"];

		wd.bulletSize = data["bulletSize"];
		wd.bulletSpeed = data["bulletSpeed"];

		wd.shootCoolTime = data["shootCoolTime"];
		wd.bulletLifeTime = data["bulletLifeTime"];

		wd.knockback = data["knockback"];

		// enum変換機能
		wd.rarity = data["rarity"].get<Rarity>();

		weaponDataMap[name] = wd;
	}
}