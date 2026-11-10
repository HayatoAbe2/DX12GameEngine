#include "EnemyManager.h"
#include "GameCommon.h"
#include "Objects/Character/Enemy/EnemyStatus.h"
#include "Objects/Weapon/WeaponManager/WeaponManager.h"
#include "Objects/Bullet/BulletManager.h"
#include "Map/MapCheck.h"
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
						itemManager->SpawnWeapon(enemy->GetTransform().translate, -1, Rarity::Common, false);
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

void EnemyManager::CreateEnemy(Vector3 pos, WeaponManager* weaponManager, const std::string& name) {
	auto it = enemyDataMap_.find(name);
	if (it == enemyDataMap_.end()) { // 見つからない
		return;
	}

	std::vector<std::unique_ptr<Enemy>> enemies;
	const EnemyData& data = it->second;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	int n = ctx.RandomInt(data.minSpawnCount, data.maxSpawnCount);
	for (int i = 0; i < n; ++i) {
		auto model = asset.LoadModel("Resources/Models/Enemy", data.modelName);
		auto shadow = asset.LoadModel("Resources/Models/Enemy", data.modelName);
		if (data.animationIndex >= 0) {
			// アニメーション割り当て(あれば)
			std::vector<std::shared_ptr<Animation>> anim = asset.LoadAnimation("Resources/Models/Enemy",data.modelName);
			if (data.animationIndex < anim.size()) {
				anim[data.animationIndex]->isLoop = true;
				model->SetAnimation(anim[data.animationIndex]);
				shadow->SetAnimation(anim[data.animationIndex]);
			}
		}

		// 位置をずらす
		Vector3 spawnPos = pos;
		if (i > 0) {
			spawnPos = pos + Vector3{ ctx.RandomFloat(-1,1), 0, ctx.RandomFloat(-1,1) };
			model->SetTranslate(spawnPos);
			shadow->SetTranslate(spawnPos);
		}

		// 武器割り当て
		std::vector<std::unique_ptr<Weapon>> weapons;
		for (auto& weaponName : data.weapons) { 
			std::unique_ptr<Weapon> weapon;
			weapon = weaponManager->GetEnemyAttack(weaponName);
			weapons.push_back(std::move(weapon));
		}
		enemies.push_back(std::make_unique<Enemy>(std::move(model), std::move(shadow), spawnPos, data.status, std::move(weapons)));
	}

	// 出現待機
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

		// 敵の生成
		std::string enemyName = random->SelectEnemy(floorManager_->GetCurrentDepth());
		if (enemyName.empty()) { // 該当なしなら出さない
			continue;
		}

		CreateEnemy({ spawnPos.x, 0, spawnPos.y }, weaponManager, enemyName);
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

		// ステータス
		enemy.status.hp = data.at("hp").get<float>();
		enemy.status.radius = data.at("radius").get<float>();
		enemy.status.moveSpeed = data.at("moveSpeed").get<float>();
		enemy.status.defaultSearchRadius = data.at("defaultSearchRadius").get<float>();
		enemy.status.stunResist = data.at("stunResist").get<float>();
		enemy.status.canFly = data.at("canFly").get<bool>();
		enemy.status.attackRadius = data.at("attackRadius").get<float>();

		// 特殊性能
		if (data.contains("traits")) {
			auto& traits = data["traits"];
			if (traits.contains("contactDamage")) {
				ContactDamage t;
				auto& j = traits["contactDamage"];
				t.damage = j["damage"];
				enemy.status.traits.contactDamage = t;
			}
		}

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
