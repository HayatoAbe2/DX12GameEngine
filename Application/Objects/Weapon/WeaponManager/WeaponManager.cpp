#include "WeaponManager.h"
#include "Objects/Weapon/Weapon.h"
#include "Data/WeaponSerializer.h"
#include <Externals/nlohmann/json.hpp>
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <Windows.h>

namespace {
	// 武器データのフォルダ(敵用は名前の末尾が _Enemy)
	constexpr const char* kWeaponDir = "Resources/Data/Weapons";
	constexpr const char* kEnemySuffix = "_Enemy";

	// デバッグ出力(Visual Studioの出力ウィンドウ)
	void Log(const std::string& message) {
		OutputDebugStringA((message + "\n").c_str());
	}
}

void WeaponManager::Initialize() {
	Reload();
}

void WeaponManager::Reload() {
	weaponDataMap.clear();
	enemyAttackDataMap.clear();

	// フォルダ内のjsonをすべて読み込む(エディタで追加した武器もここで登録される)
	std::vector<std::filesystem::path> files;
	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(kWeaponDir, ec)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			files.push_back(entry.path());
		}
	}
	std::sort(files.begin(), files.end());

	for (const auto& path : files) {
		LoadJson(path.string());
	}
}

std::vector<std::string> WeaponManager::GetWeaponNames() const {
	std::vector<std::string> names;
	names.reserve(weaponDataMap.size());
	for (const auto& [name, data] : weaponDataMap) {
		names.push_back(name);
	}
	std::sort(names.begin(), names.end());
	return names;
}

std::vector<std::string> WeaponManager::GetEnemyAttackNames() const {
	const std::string suffix = kEnemySuffix;
	std::vector<std::string> names;
	names.reserve(enemyAttackDataMap.size());
	for (const auto& [key, data] : enemyAttackDataMap) {
		// GetEnemyAttack(name) に渡す名前は末尾の _Enemy を除いたもの
		names.push_back(key.substr(0, key.size() - suffix.size()));
	}
	std::sort(names.begin(), names.end());
	return names;
}

const WeaponData* WeaponManager::GetEnemyAttackData(const std::string& name) const {
	const auto it = enemyAttackDataMap.find(name + kEnemySuffix);
	return it != enemyAttackDataMap.end() ? &it->second : nullptr;
}

std::unique_ptr<Weapon> WeaponManager::CreateWeapon(const WeaponData& data) {
	auto& asset = GameContext::GetInstance().Asset();

	std::unique_ptr<Model> model = asset.LoadModel("Resources/Models/Weapons", data.modelName);
	std::unique_ptr<Model> shadowModel = asset.LoadModel("Resources/Models/Weapons", data.modelName);
	MaterialData matData = model->GetMaterial(0)->GetData();
	matData.color = { 0.2f,0.2f,0.2f,1 };
	model->GetMaterial(0)->SetData(matData);
	model->GetMaterial(1)->SetData(matData);

	return std::make_unique<Weapon>(data, std::move(model), std::move(shadowModel));
}

std::unique_ptr<Weapon> WeaponManager::GetWeapon(int index, Rarity rarity) {
	auto& ctx = GameContext::GetInstance();

	// ランダム(読み込まれている武器の中から)
	if (index == -1) {
		const std::vector<std::string> names = GetWeaponNames();
		if (names.empty()) { return nullptr; }
		return GetWeaponByName(names[ctx.RandomInt(0, int(names.size()) - 1)], rarity);
	}

	if (index < 0 || index >= int(WEAPON::Count)) {
		Log(std::format("[WeaponManager] GetWeapon: invalid index {}", index));
		return nullptr;
	}
	return GetWeaponByName(WeaponNames[index], rarity);
}

std::unique_ptr<Weapon> WeaponManager::GetWeaponByName(const std::string& name, Rarity rarity) {
	const auto it = weaponDataMap.find(name);
	if (it == weaponDataMap.end()) {
		Log(std::format("[WeaponManager] GetWeaponByName: weapon not found: {}", name));
		return nullptr;
	}
	return CreateWeapon(it->second);
}

std::unique_ptr<Weapon> WeaponManager::GetEnemyAttack(int index) {
	auto& ctx = GameContext::GetInstance();

	// ランダム
	if (index == -1) {
		const std::vector<std::string> names = GetEnemyAttackNames();
		if (names.empty()) { return nullptr; }
		return GetEnemyAttack(names[ctx.RandomInt(0, int(names.size()) - 1)]);
	}

	if (index < 0 || index >= int(ENEMYATTACK::Count)) {
		Log(std::format("[WeaponManager] GetEnemyAttack: invalid index {}", index));
		return nullptr;
	}
	return GetEnemyAttack(EnemyAttackNames[index]);
}

std::unique_ptr<Weapon> WeaponManager::GetEnemyAttack(const std::string& name) {
	// JSONのキーと同じ名前
	const std::string key = name + kEnemySuffix;

	const auto it = enemyAttackDataMap.find(key);
	if (it == enemyAttackDataMap.end()) {
		Log(std::format("[WeaponManager] GetEnemyAttack: weapon not found: {}", key));
		return nullptr;
	}
	return CreateWeapon(it->second);
}

void WeaponManager::LoadJson(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		Log(std::format("[WeaponManager] cannot open: {}", path));
		return;
	}

	nlohmann::json j;
	try {
		file >> j;
	} catch (const std::exception& e) {
		Log(std::format("[WeaponManager] parse error: {} ({})", path, e.what()));
		return;
	}
	if (!j.is_object()) { return; }

	for (const auto& [name, data] : j.items()) {
		WeaponData weapon = WeaponSerializer::FromJson(name, data);

		// 名前の末尾が _Enemy なら敵用
		if (WeaponSerializer::IsEnemyWeapon(name)) {
			enemyAttackDataMap[name] = weapon;
		} else {
			weaponDataMap[name] = weapon;
		}
	}
}
