#pragma once
#include "GameCommon.h"

class Camera;

class MapTile {
public:
	enum Tile {
		None,
		Floor,
		LeftWall,
		RightWall,
		UpWall,
		BottomWall,
		Goal,
		CombatWall
	};

	~MapTile();
	void Initialize(std::unique_ptr<InstancedModel> wall, std::unique_ptr<InstancedModel> floor, std::unique_ptr<InstancedModel> barrier, std::unique_ptr<InstancedModel> weaponSpawn, std::unique_ptr<InstancedModel> enemySpawn, std::unique_ptr<Model> goal);
	void UpdateMapChange();
	void LoadCSV(const std::string& filePath);
	void Update(bool canGoal);
	void Draw(Camera* camera, bool isCombat);

	std::vector<std::vector<Tile>> GetMap() { return map_; }
	const std::vector<AABB2D>& GetEnemySpawnArea() { return enemySpawnArea_; }
	const std::vector<Vector3>& GetWeaponSpawnPoints() { return weaponSpawnPoints_; }
	int GetWidth() { return mapWidth_; }
	int GetHeight() { return mapHeight_; }
	float GetTileSize() { return tileSize_; }

private:
	std::vector<std::vector<Tile>> map_;
	std::vector<AABB2D> enemySpawnArea_;
	std::vector<Vector3> weaponSpawnPoints_;
	float tileSize_ = 1.5f;
	int mapWidth_ = 0;
	int mapHeight_ = 0;

	std::unique_ptr<InstancedModel> wall_ = nullptr;
	std::unique_ptr<InstancedModel> barrier_ = nullptr;
	std::unique_ptr<InstancedModel> floor_ = nullptr;
	std::unique_ptr<InstancedModel> enemySpawn_ = nullptr;
	std::unique_ptr<InstancedModel> weaponSpawn_ = nullptr;
	std::unique_ptr<Model> goal_ = nullptr;

	std::unique_ptr<ParticleSystem> particle_ = nullptr;
	const int particleNum_ = 30;
	float particleRange_ = 3.0f;
	int emitTimer_ = 0;
	const int emitTime_ = 5;
	bool soundPlayed_ = false;

	// ライトのインデックス
	int lightIndex_ = -1;
};

