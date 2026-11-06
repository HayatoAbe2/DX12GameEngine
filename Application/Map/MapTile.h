#pragma once
#include "GameCommon.h"

class Camera;

// マップタイル
enum class Tile {
	None,
	Floor,
	Wall,
	Goal,
	CombatWall
};

/// <summary>
/// マップ作成
/// </summary>
class MapTile {
public:

	~MapTile();
	void Initialize();
	// シーンデータからマップ更新
	void UpdateMapChange(bool isCombat, bool isEditMode);
	void Update(bool canGoal);
	void Draw(Camera* camera);

	std::vector<std::vector<Tile>> GetMap() { return map_; }
	int GetWidth() { return mapWidth_; }
	int GetHeight() { return mapHeight_; }
	float GetTileSize() { return tileSize_; }

private:
	std::vector<std::vector<Tile>> map_;
	float tileSize_ = 1.5f;
	int mapWidth_ = 64;
	int mapHeight_ = 64;
	
	// パーティクル
	Transform particleTransform_{};
	std::unique_ptr<ParticleSystem> particle_ = nullptr;
	const int particleNum_ = 30;
	float particleRange_ = 3.0f;
	int emitTimer_ = 0;
	const int emitTime_ = 5;
	bool soundPlayed_ = false;

	// ライトのインデックス
	int lightIndex_ = -1;
};

