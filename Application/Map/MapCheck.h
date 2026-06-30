#pragma once
#define NOMINMAX
#include "MapTile.h"
#include "GameCommon.h"

class Player;

class MapCheck {
public:
	void Initialize(std::vector<std::vector<MapTile::Tile>> map, float tileSize);
	void Update(std::vector<std::vector<MapTile::Tile>> map);
	bool ResolveCollisionX(Vector2& pos, float radius, bool isFlying);
	bool ResolveCollisionY(Vector2& pos, float radius, bool isFlying);
	bool IsHitWall(const Vector2& pos, float radius);
	bool IsFall(const Vector2& pos);
	bool IsGoal(const Vector2& pos, float radius, bool canGoal);
	bool EnemyCanSeePlayer(const Vector3& enemyPos, const Vector3& playerPos);

	void SetCombat(bool isCombat) { isCombat_ = isCombat; }

private:	
	std::vector<std::vector<MapTile::Tile>> map_;
	float tileSize_ = 0;
	bool isCombat_ = false;
};

