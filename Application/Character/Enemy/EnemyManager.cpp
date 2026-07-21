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
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Engine/SceneObject/SceneObject.h"

void EnemyManager::Initialize() {
}

void EnemyManager::Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, Camera* camera) {
	for (const auto& enemy : activeEnemies_) {
		enemy->Update(mapCheck, player, bulletManager, camera);
	}

	activeEnemies_.erase(
		std::remove_if(activeEnemies_.begin(), activeEnemies_.end(),
			[](const std::unique_ptr<Enemy>& enemy) {
				return enemy->IsDead();
			}
		),
		activeEnemies_.end()
	);
}


void EnemyManager::Draw(Camera* camera) {
	for (const auto& enemy : activeEnemies_) {
		enemy->Draw(camera);
	}
}

void EnemyManager::Spawn(std::vector<std::unique_ptr<Enemy>> enemy) {
	// 控えの敵を出現させる
	for (auto& e : enemy) {
		activeEnemies_.push_back(std::move(e));
	}
}

void EnemyManager::CreateEnemy(Vector3 pos, WeaponManager* weaponManager, int enemyType) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	std::vector<std::unique_ptr<Weapon>> weapons;
	EnemyStatus status;

	std::vector<std::unique_ptr<Enemy>> enemies;
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
		status.moveSpeed = 3.0f;
		status.defaultSearchRadius = 5.0f;
		status.stunResist = 0;
		status.canFly = true;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<Bat>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case 2:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "knight.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "knight.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Pistol)));
		status.hp = 23;
		status.radius = 0.9f;
		status.moveSpeed = 1.5f;
		status.defaultSearchRadius = 10.0f;
		status.stunResist = 2;
		status.canFly = false;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<Knight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}
	case 3:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "knight2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "knight2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Pistol)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Pistol)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::FireBall)));
		status.hp = 150;
		status.radius = 1.5f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 30;
		status.canFly = false;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<HeavyKnight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case 4:
	{
		auto enemyModel = asset.LoadModel("Resources/Enemy", "bat2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "bat2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::FireBall)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::AssaultRifle)));
		weapons.push_back(weaponManager->GetWeapon(int(WeaponManager::WEAPON::Wavegun)));
		status.hp = 110;
		status.radius = 0.75f;
		status.moveSpeed = 7.0f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 30;
		status.canFly = true;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<RedBat>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}
	default:
	{
		status.hp = 5;
		status.radius = 0.5f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 8.5f;
		status.stunResist = 0;
		status.canFly = false;
		status.attackRadius = -1;

		int num = ctx.RandomInt(2, 4);
		for (int i = 0; i < num; ++i) {
			auto enemyModel = asset.LoadModel("Resources/Enemy", "hedgehog.obj");
			auto enemyShadowModel = asset.LoadModel("Resources/Enemy", "hedgehog.obj");
			Vector3 spawnPos = pos + Vector3{ ctx.RandomFloat(-1,1), 0, ctx.RandomFloat(-1,1) };
			enemyModel->SetTranslate(spawnPos);
			enemyShadowModel->SetTranslate(spawnPos);

			enemies.push_back(std::make_unique<Spiker>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		}
		break;
	}
	}

	inactiveEnemies_.push_back(std::move(enemies));
}

void EnemyManager::Reset() {
	activeEnemies_.clear();
	isSpawned_.clear();
}

void EnemyManager::Load(WeaponManager* weaponManager) {
	isSpawned_.clear();
	spawnArea_.clear();
	spawnPos_.clear();
	activeEnemies_.clear();
	inactiveEnemies_.clear();

	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	// モデル座標をスポーン位置に反映
	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);
			models.push_back(model);
		}
	}

	for (auto& model : models) {
		if (model->tag == "enemySpawn") {
			for (auto& t : model->GetTransforms()) {
				if (t.scale.x != 0 ||
					t.scale.y != 0 ||
					t.scale.z != 0) {
					AABB2D aabb = { {t.translate.x - t.scale.x / 2.0f, t.translate.z - t.scale.z / 2.0f }, {t.translate.x + t.scale.x / 2.0f, t.translate.z + t.scale.z / 2.0f} };
					spawnArea_.push_back(aabb);
				}
			}
		} else if (model->tag == "enemySpawnPoint") {
			for (auto& t : model->GetTransforms()) { 
				if (t.scale.x != 0 ||
					t.scale.y != 0 ||
					t.scale.z != 0) {

					// 出現座標追加
					spawnPos_.push_back(ToXZ(t.translate));
					isSpawned_.push_back(false);

					// 敵No.
					int num = ctx.RandomInt(1, 3);
					if (num == 3) num = 5;

					// 出現敵のロード
					CreateEnemy({ t.translate.x, 0, t.translate.z }, weaponManager, num);
				}
			}
		}
	}
}

void EnemyManager::SpawnCheck(const Vector3& playerPos, MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();

	for (int i = 0; i < spawnArea_.size(); ++i)
		for (int j = 0; j < isSpawned_.size(); ++j) {
			if (!isSpawned_[j]) {
				// 入ったら出現
				if (CheckCollision(spawnArea_[i], ToXZ(playerPos))) {

					// 範囲内の出現ポイント
					if (CheckCollision(spawnArea_[i], spawnPos_[j])) {
						Spawn(std::move(inactiveEnemies_[j]));
						isSpawned_[j] = true;
					}
				}
			}
		}
}

std::vector<Enemy*> EnemyManager::GetEnemies() {
	std::vector<Enemy*> result;
	result.reserve(activeEnemies_.size());
	for (auto& e : activeEnemies_) {
		result.push_back(e.get());
	}
	return result;
}
