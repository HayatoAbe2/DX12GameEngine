#pragma once
#include "GameCommon.h"
#include "Enemy.h"

class MapCheck;
class Player;
class BulletManager;
class WeaponManager;

class EnemyManager {
public:
	void Initialize();
	void Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, Camera* camera);
	void Draw(Camera* camera);
	void Spawn(Vector3 pos, WeaponManager* weaponManager, int enemyType);
	void Reset();
	void LoadCSV(std::string filePath, float tileSize, WeaponManager* weaponManager);

	std::vector<Enemy*> GetEnemies();
private:
	std::vector<std::unique_ptr<Enemy>> enemies_ = {};

};

