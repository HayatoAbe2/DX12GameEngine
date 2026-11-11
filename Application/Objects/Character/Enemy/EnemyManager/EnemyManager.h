#pragma once
#include "GameCommon.h"
#include "Objects/Character/Enemy/Enemy.h"
#include "RandomSettings/RandomSettings.h"
#include "Map/FloorManager/FloorManager.h"

class MapCheck;
class Player;
class BulletManager;
class ItemManager;
class WeaponManager;

/// <summary>
/// 敵の管理
/// </summary>
class EnemyManager {
public:
	EnemyManager(FloorManager* floorManager) { floorManager_ = floorManager; LoadEnemyData(); }

	// 登録されている敵の名前(ソート済み)
	std::vector<std::string> GetEnemyNames() const;

	// 敵データ(Enemies.json)を読み込み直す。次に出現する敵から反映される
	void ReloadEnemyData() { LoadEnemyData(); }

	void Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, ItemManager* itemManager);
	void Draw();

	// 敵スポーン
	void Spawn(std::vector<std::unique_ptr<Enemy>> enemy);

	// 敵を抽選して出現予約
	void CreateEnemy(Vector3 pos, WeaponManager* weaponManager, const std::string& name);

	// 次の更新で、その場にすぐ出現させる(デバッグ・エディタ用。名前は GetEnemyNames() のもの)
	void RequestSpawn(Vector3 pos, WeaponManager* weaponManager, const std::string& name);
	void Reset();
	void Load(WeaponManager* weaponManager, RandomSettings* random);
	void SpawnCheck(const Vector3& playerPos, MapCheck* mapCheck);

	std::vector<Enemy*> GetEnemies();
private:
	void LoadEnemyData();

	// 名前から敵を生成する(見つからなければ空)
	std::vector<std::unique_ptr<Enemy>> MakeEnemies(Vector3 pos, WeaponManager* weaponManager, const std::string& name);

	FloorManager* floorManager_ = nullptr;

	std::vector<std::unique_ptr<Enemy>> activeEnemies_ = {};
	std::vector<std::vector<std::unique_ptr<Enemy>>> inactiveEnemies_ = {};

	std::unique_ptr<ParticleSystem> deathParticle_ = nullptr;

	std::vector<bool> isSpawned_;
	std::vector<AABB2D> spawnArea_;
	std::vector<Vector2> spawnPos_;

	// 敵データ
	std::unordered_map<std::string, EnemyData> enemyDataMap_;

	// すぐ出現させる予約(生成を描画の途中で行わないため、次の更新で処理する)
	struct SpawnRequest {
		Vector3 pos;
		WeaponManager* weaponManager = nullptr;
		std::string name;
	};
	std::vector<SpawnRequest> spawnRequests_;
};

