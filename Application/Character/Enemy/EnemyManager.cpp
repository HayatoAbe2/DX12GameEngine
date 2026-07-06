#include "EnemyManager.h"
#include "GameCommon.h"
#include "Enemy.h"
#include "Weapon/WeaponManager.h"
#include "Bullet/BulletManager.h"
#include "EnemyStatus.h"
#include "Map/MapCheck.h"

#include "Character/Enemy/Enemies/Bat.h"
#include "Character/Enemy/Enemies/Knight.h"
#include "Character/Enemy/Enemies/HeavyKnight.h"
#include "Character/Enemy/Enemies/RedBat.h"
#include "Character/Enemy/Enemies/Spiker.h"
#include <fstream>
#include <sstream>
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Engine/SceneObject/SceneObject.h"

void EnemyManager::Initialize() {
}

void EnemyManager::Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, Camera* camera) {
	for (const auto& enemy : enemies_) {
		enemy->Update(mapCheck, player, bulletManager, camera);
	}

	enemies_.erase(
		std::remove_if(enemies_.begin(), enemies_.end(),
			[](const std::unique_ptr<Enemy>& enemy) {
				return enemy->IsDead();
			}
		),
		enemies_.end()
	);
}


void EnemyManager::Draw(Camera* camera) {
	for (const auto& enemy : enemies_) {
		enemy->Draw(camera);
	}
}

void EnemyManager::Spawn(Vector3 pos, WeaponManager* weaponManager, int enemyType) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();


	std::vector<std::unique_ptr<Weapon>> weapons;
	EnemyStatus status;

	switch (enemyType) {
	case 1:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "bat.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "bat.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::FireBall)));
		status.hp = 10;
		status.radius = 0.5f;
		status.moveSpeed = 7.0f;
		status.defaultSearchRadius = 5.0f;
		status.stunResist = 0;
		status.canFly = true;
		status.attackRadius = 9;

		enemies_.push_back(std::make_unique<Bat>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case 2:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "knight.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "knight.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::AssaultRifle)));
		status.hp = 20;
		status.radius = 0.9f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 10.0f;
		status.stunResist = 2;
		status.canFly = false;
		status.attackRadius = 9;

		enemies_.push_back(std::make_unique<Knight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}
	case 3:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "knight2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "knight2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::AssaultRifle)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::AssaultRifle)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::FireBall)));
		status.hp = 150;
		status.radius = 1.5f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 30;
		status.canFly = false;
		status.attackRadius = 9;

		enemies_.push_back(std::make_unique<HeavyKnight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case 4:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "bat2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "bat2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::FireBall)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Pistol)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Wavegun)));
		status.hp = 110;
		status.radius = 0.75f;
		status.moveSpeed = 7.0f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 30;
		status.canFly = true;
		status.attackRadius = 9;

		enemies_.push_back(std::make_unique<RedBat>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}
	default:
	{
		status.hp = 10;
		status.radius = 0.5f;
		status.moveSpeed = 4.0f;
		status.defaultSearchRadius = 8.5f;
		status.stunResist = 0;
		status.canFly = false;
		status.attackRadius = -1;
		for (int i = 0; i < 5; ++i) {
			auto enemyModel = asset.LoadModel("Resources/Enemy", "hedgehog.obj");
			auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "hedgehog.obj");
			Vector3 spawnPos = pos + Vector3{ ctx.RandomFloat(-1,1), 0, ctx.RandomFloat(-1,1) };
			enemyModel->SetTranslate(spawnPos);
			enemyShadowModel->SetTranslate(spawnPos);

			enemies_.push_back(std::make_unique<Spiker>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		}
		break;
	}
	}
}

void EnemyManager::Reset() {
	enemies_.clear();
	isSpawned_.clear();
}

void EnemyManager::LoadCSV(std::string filePath, float tileSize, WeaponManager* weaponManager) {
	std::ifstream file(filePath);
	std::string line;

	assert(file.is_open());

	std::getline(file, line); // 最初の行をスキップ

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string enemyStr, xStr, zStr;

		std::getline(ss, enemyStr, ',');
		std::getline(ss, xStr, ',');
		std::getline(ss, zStr, ',');

		int enemyNum = std::stoi(enemyStr);
		float x = std::stof(xStr);
		float z = std::stof(zStr);

		Vector3 pos = Vector3{ x * tileSize, 0, z * tileSize };
		Spawn(pos, weaponManager, enemyNum);
	}
}

void EnemyManager::Load(WeaponManager* weaponManager, const Vector3& playerPos, MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);
			models.push_back(model);
		}
	}

	std::vector<AABB2D> spawnArea;
	std::vector<Vector3> spawnPos;
	for (auto& model : models) {
		if (model->tag == "enemySpawn") {
			if (isSpawned_.size() < model->GetTransforms().size()) {
				isSpawned_.resize(model->GetTransforms().size());
			}

			for (int i = 0; i < model->GetTransforms().size(); ++i) {
				Transform t = model->GetTransforms()[i];
				AABB2D aabb = { {t.translate.x - t.scale.x / 2.0f, t.translate.z - t.scale.z / 2.0f }, {t.translate.x + t.scale.x / 2.0f, t.translate.z + t.scale.z / 2.0f} };
				spawnArea.push_back(aabb);
			}

		} else if (model->tag == "enemySpawnPoint") {
			for (int i = 0; i < model->GetTransforms().size(); ++i) {
				Transform t = model->GetTransforms()[i];
				spawnPos.push_back(t.translate);
			}
		}
	}

	for (int i = 0; i < isSpawned_.size(); ++i) {
		if (!isSpawned_[i]) {
			// 入ったら出現
			if (CheckCollision(spawnArea[i], Vector2{ playerPos.x, playerPos.z })) {

				for (Vector3 pos : spawnPos) {
					// 範囲内の出現ポイント
					if (CheckCollision(spawnArea[i], Vector2{ pos.x, pos.z })) {
						int num = ctx.RandomInt(1, 3);
						if (num == 3) num = 5;

						Spawn(pos, weaponManager, num);
					}
				}
				isSpawned_[i] = true;
			}
		}
	}

}

std::vector<Enemy*> EnemyManager::GetEnemies() {
	std::vector<Enemy*> result;
	result.reserve(enemies_.size());
	for (auto& e : enemies_) {
		result.push_back(e.get());
	}
	return result;
}
