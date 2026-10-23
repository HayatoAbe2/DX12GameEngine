#include "EnemyManager.h"
#include "GameCommon.h"
#include "Objects/Character/Enemy/EnemyStatus.h"
#include "Objects/Weapon/WeaponManager/WeaponManager.h"
#include "Objects/Bullet/BulletManager.h"
#include "Map/MapCheck.h"

#include "Objects/Character/Enemy/Enemies/Bat.h"
#include "Objects/Character/Enemy/Enemies/Knight.h"
#include "Objects/Character/Enemy/Enemies/HeavyKnight.h"
#include "Objects/Character/Enemy/Enemies/RedBat.h"
#include "Objects/Character/Enemy/Enemies/Spiker.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Engine/SceneObject/SceneObject.h"
#include "Objects/Item/ItemManager.h"

#include <fstream>
#include <cassert>
#include <Externals/nlohmann/json.hpp>

void EnemyManager::Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, ItemManager* itemManager) {
	for (const auto& enemy : activeEnemies_) {
		enemy->Update(mapCheck, player, bulletManager);
	}

	activeEnemies_.erase(
		std::remove_if(activeEnemies_.begin(), activeEnemies_.end(),
			[&](const std::unique_ptr<Enemy>& enemy) {
				if (enemy->IsDead()) {
					itemManager->SpawnMoney(enemy->GetTransform().translate, 15);

					if (GameContext::GetInstance().RandomInt(0, 100) >= 95) {
						itemManager->SpawnWeapon(enemy->GetTransform().translate, -1, Common, false);
					}

					return true;
				}
				return false;
			}
		),
		activeEnemies_.end()
	);
}


void EnemyManager::Draw() {
	for (const auto& enemy : activeEnemies_) {
		enemy->Draw();
	}
}

void EnemyManager::Spawn(std::vector<std::unique_ptr<Enemy>> enemy) {
	// 控えの敵を出現させる
	for (auto& e : enemy) {
		activeEnemies_.push_back(std::move(e));
	}
}


static std::string GetEnemyDataKey(EnemyType type) {
	// ファイル内での敵名
	switch (type) {
	case EnemyType::Fly:
		return "Fly";

	case EnemyType::Golem:
		return "Golem";

	case EnemyType::Hedgehog_Single:
		return "Hedgehog_Single";

	case EnemyType::elite1:
		return "elite1";

	case EnemyType::elite2:
		return "elite2";

	default:
		return "Hedgehog_Group";
	}
}

void EnemyManager::CreateEnemy(Vector3 pos, WeaponManager* weaponManager, EnemyType enemyType) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	std::vector<std::unique_ptr<Weapon>> weapons;
	EnemyStatus status;

	std::vector<std::unique_ptr<Enemy>> enemies;
	switch (enemyType) {
	case EnemyType::Fly:
	{
		auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "Dragon.glb");
		auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "Dragon.glb");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::Fireball)));

		status.hp = 10;
		status.radius = 0.5f;
		status.moveSpeed = 5.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 0;
		status.canFly = true;
		status.attackRadius = 9;

		auto anim = asset.LoadAnimation("Resources/Models/Enemy", "Dragon.glb")[2];
		anim->isRoop = true;
		enemyModel->SetAnimation(anim);
		enemyShadowModel->SetAnimation(anim);

		enemies.push_back(std::make_unique<Bat>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case EnemyType::Golem:
	{
		auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "knight.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "knight.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::Pistol)));
		status.hp = 23;
		status.radius = 0.9f;
		status.moveSpeed = 1.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 2;
		status.canFly = false;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<Knight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}
	case EnemyType::Hedgehog_Single:
	{
		status.hp = 5;
		status.radius = 0.5f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 0;
		status.canFly = false;
		status.attackRadius = -1;

		auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "hedgehog.glb");
		auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "hedgehog.glb");
		Vector3 spawnPos = pos;
		enemyModel->SetTranslate(spawnPos);
		enemyShadowModel->SetTranslate(spawnPos);

		auto anim = asset.LoadAnimation("Resources/Models/Enemy", "hedgehog.glb")[2];
		anim->isRoop = true;
		enemyModel->SetAnimation(anim);
		enemyShadowModel->SetAnimation(anim);

		enemies.push_back(std::make_unique<Spiker>(std::move(enemyModel), std::move(enemyShadowModel), spawnPos, status, std::move(weapons)));
	}
	break;
	case EnemyType::elite1:
	{
		auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "knight2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "knight2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::ThreeWay)));
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::FourWay)));
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::LargeFireball)));
		status.hp = 200;
		status.radius = 1.5f;
		status.moveSpeed = 3.5f;
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 30;
		status.canFly = false;
		status.attackRadius = 9;

		enemies.push_back(std::make_unique<HeavyKnight>(std::move(enemyModel), std::move(enemyShadowModel), pos, status, std::move(weapons)));
		break;
	}

	case EnemyType::elite2:
	{
		auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "bat2.obj");
		auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "bat2.obj");
		enemyModel->SetTranslate(pos);
		enemyShadowModel->SetTranslate(pos);
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::Fireball)));
		weapons.push_back(weaponManager->GetEnemyAttack(int(WeaponManager::ENEMYATTACK::AssaultRifle)));
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
		status.defaultSearchRadius = 100.0f;
		status.stunResist = 0;
		status.canFly = false;
		status.attackRadius = -1;

		int num = ctx.RandomInt(2, 4);
		for (int i = 0; i < num; ++i) {
			auto enemyModel = asset.LoadModel("Resources/Models/Enemy", "hedgehog.glb");
			auto enemyShadowModel = asset.LoadModel("Resources/Models/Enemy", "hedgehog.glb");
			Vector3 spawnPos = pos + Vector3{ ctx.RandomFloat(-1,1), 0, ctx.RandomFloat(-1,1) };
			enemyModel->SetTranslate(spawnPos);
			enemyShadowModel->SetTranslate(spawnPos);

			auto anim = asset.LoadAnimation("Resources/Models/Enemy", "hedgehog.glb")[2];
			anim->isRoop = true;
			enemyModel->SetAnimation(anim);
			enemyShadowModel->SetAnimation(anim);

			enemies.push_back(std::make_unique<Spiker>(std::move(enemyModel), std::move(enemyShadowModel), spawnPos, status, std::move(weapons)));
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

void EnemyManager::Load(WeaponManager* weaponManager, RandomSettings* random) {
	LoadEnemyData();

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

	std::vector<Vector2> positions; // 全ての出現位置
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
				if (t.scale == Vector3{ 0,0,0 }) {
					continue;
				}

				// 出現座標追加
				positions.push_back(ToXZ(t.translate));
			}
		}
	}

	// 出現数
	int spawnCount = 3;
	if (floorManager_->GetCurrentDepth() > 2) {
		spawnCount++;
	}
	if (floorManager_->GetCurrentDepth() > 4) {
		spawnCount++;
	}
	if (floorManager_->GetCurrentDepth() > 6) {
		spawnCount++;
	}
	spawnCount = min(spawnCount, int(positions.size()));
	if (floorManager_->GetCurrentDepth() == 9) { spawnCount = 1; }

	for (size_t i = 0; i < spawnCount; ++i) {
		// 残っている候補からランダムに1つ選ぶ
		const int index = ctx.RandomInt(0, int(positions.size()) - 1);
		const auto spawnPos = positions[index];

		// 重複を防ぐため削除
		positions.erase(positions.begin() + index);

		// 実際に出現させる地点
		spawnPos_.push_back(spawnPos);
		isSpawned_.push_back(false);

		CreateEnemy({ spawnPos.x, 0, spawnPos.y }, weaponManager, random->SelectEnemy(floorManager_->GetCurrentDepth()));
	}
}

void EnemyManager::SpawnCheck(const Vector3& playerPos, MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();

	for (int i = 0; i < spawnArea_.size(); ++i) {
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
}

std::vector<Enemy*> EnemyManager::GetEnemies() {
	std::vector<Enemy*> result;
	result.reserve(activeEnemies_.size());
	for (auto& e : activeEnemies_) {
		result.push_back(e.get());
	}
	return result;
}

void EnemyManager::LoadEnemyData() {
	const std::string path = "Resources/Data/Enemies/Enemies.json";
	std::ifstream file(path);

	nlohmann::json j;
	file >> j;

	enemyDataMap_.clear();

	for (const auto& [name, data] : j.items()) {
		EnemyData enemy{};

		enemy.modelName = data.at("modelName").get<std::string>();
		enemy.animationIndex = data.value("animationIndex", -1);

		enemy.hp = data.at("hp").get<int>();
		enemy.radius = data.at("radius").get<float>();
		enemy.moveSpeed = data.at("moveSpeed").get<float>();
		enemy.defaultSearchRadius =
			data.at("defaultSearchRadius").get<float>();
		enemy.stunResist = data.at("stunResist").get<int>();
		enemy.canFly = data.at("canFly").get<bool>();
		enemy.attackRadius = data.at("attackRadius").get<float>();

		if (data.contains("weapons")) {
			enemy.weapons = data.at("weapons").get<std::vector<std::string>>();
		}

		if (data.contains("spawnCount")) {
			const auto& count = data.at("spawnCount");
			enemy.minSpawnCount = count.value("min", 1);
			enemy.maxSpawnCount = count.value("max", 1);
		}

		enemyDataMap_.emplace(name, std::move(enemy));
	}
}
