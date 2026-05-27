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
	LoadJson("Resources/Weapons/Data/pistol.json"); 
	// まだ反映はしてない
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
	WeaponStatus status;

	// エンチャント付与
	status.rarity = rarity;
	for (int i = 0; i < static_cast<int>(status.rarity); ++i) {
		status.enchants.push_back(static_cast<Enchants>(ctx.RandomInt(0, enchantsCount-1)));
	}

	if (index == -1) { index = ctx.RandomInt(0, int(WEAPON::Wavegun)); }
	MaterialData matData = {};
	switch (index) {
	case static_cast<int>(WEAPON::Pistol):
		status.damage = 3.0f;
		status.weight = 0.1f;
		status.bulletSize = 0.5f;
		status.bulletSpeed = 0.6f;
		status.shootCoolTime = 20;
		status.bulletLifeTime = 60;
		status.knockback = 0.3f;
		status.magazineSize = 10;
		status.reloadTime = 30;
		model = asset.LoadModel("Resources/Weapons", "Pistol.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "Pistol.obj");
		return std::make_unique<Pistol>(status, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::AssaultRifle):
		status.damage = 1.2f;
		status.weight = 0.4f;
		status.bulletSize = 0.3f;
		status.bulletSpeed = 1.2f;
		status.shootCoolTime = 5;
		status.bulletLifeTime = 60;
		status.knockback = 0.1f;
		status.magazineSize = 16;
		status.reloadTime = 75;
		model = asset.LoadModel("Resources/Weapons", "AssaultRifle.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "AssaultRifle.obj");
		return std::make_unique<AssaultRifle>(status, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::Shotgun):
		status.damage = 3.0f;
		status.weight = 0.5f;
		status.bulletSize = 0.4f;
		status.bulletSpeed = 0.9f;
		status.shootCoolTime = 40;
		status.bulletLifeTime = 9;
		status.knockback = 0.3f;
		status.magazineSize = 7;
		status.reloadTime = 90;
		model = asset.LoadModel("Resources/Weapons", "Shotgun.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "Shotgun.obj");
		return std::make_unique<Shotgun>(status, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::FireBall):
		status.damage = 8.0f;
		status.weight = 0.2f;
		status.bulletSize = 1.2f;
		status.bulletSpeed = 0.4f;
		status.shootCoolTime = 60;
		status.bulletLifeTime = 300;
		status.knockback = 0.5f;
		status.magazineSize = -1;
		status.reloadTime = 0;
		model = asset.LoadModel("Resources/Weapons", "Spellbook.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "Spellbook.obj");
		return std::make_unique<FireBall>(status, std::move(model), std::move(shadowModel));
		break;

	case static_cast<int>(WEAPON::Wavegun):
		status.damage = 2.0f;
		status.weight = 0.1f;
		status.bulletSize = 0.4f;
		status.bulletSpeed = 0.4f;
		status.shootCoolTime = 10;
		status.bulletLifeTime = 180;
		status.knockback = 0.25f;
		status.magazineSize = 9;
		status.reloadTime = 50;
		model = asset.LoadModel("Resources/Weapons", "Wavegun.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "Wavegun.obj");
		return std::make_unique<Wavegun>(status, std::move(model), std::move(shadowModel));
		break;

	default:
		status.damage = 3.0f;
		status.weight = 0.1f;
		status.bulletSize = 0.5f;
		status.bulletSpeed = 0.6f;
		status.shootCoolTime = 20;
		status.bulletLifeTime = 60;
		status.knockback = 0.3f;
		model = asset.LoadModel("Resources/Weapons", "Pistol.obj");
		matData = model->GetMaterial(0)->GetData();
		matData.color = { 0.2f,0.2f,0.2f,1 };
		model->GetMaterial(0)->SetData(matData);
		model->GetMaterial(1)->SetData(matData);
		shadowModel = asset.LoadModel("Resources/Weapons", "Pistol.obj");
		return std::make_unique<Pistol>(status, std::move(model), std::move(shadowModel));
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

		wd.magazineSize = data["magazineSize"];
		wd.reloadTime = data["reloadTime"];

		weaponDataMap[name] = wd;
	}
}