#pragma once
#include "GameCommon.h"
#include "Enemy.h"

class MapCheck;
class Player;
class BulletManager;
class ItemManager;
class WeaponManager;

class EnemyManager {
public:
	void Initialize();
	void Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, ItemManager* itemManager);
	void Draw();
	void Spawn(std::vector<std::unique_ptr<Enemy>> enemy);
	void CreateEnemy(Vector3 pos, WeaponManager* weaponManager, int enemyType);
	void Reset();
	void Load(WeaponManager* weaponManager);
	void SpawnCheck(const Vector3& playerPos, MapCheck* mapCheck);

	std::vector<Enemy*> GetEnemies();
private:
	std::vector<std::unique_ptr<Enemy>> activeEnemies_ = {};
	std::vector<std::vector<std::unique_ptr<Enemy>>> inactiveEnemies_ = {};

	std::unique_ptr<ParticleSystem> deathParticle_ = nullptr;

	std::vector<bool> isSpawned_;
	std::vector<AABB2D> spawnArea_;
	std::vector<Vector2> spawnPos_;
};

