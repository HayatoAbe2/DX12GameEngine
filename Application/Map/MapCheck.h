#pragma once
#define NOMINMAX
#include "MapTile.h"
#include "GameCommon.h"
#include "Timer/Timer.h"

class Player;

/// <summary>
/// マップとの衝突判定
/// </summary>
class MapCheck {
public:
	MapCheck(const std::vector<std::vector<Tile>>& map, float tileSize);
	void Reset(const std::vector<std::vector<Tile>>& map, float tileSize);

	// 変更の反映
	void Update(const std::vector<std::vector<Tile>>& map);

	// 衝突判定と座標修正
	bool ResolveCollisionX(Vector2& pos, float radius, bool isFlying);
	bool ResolveCollisionY(Vector2& pos, float radius, bool isFlying);

	// 壁に接触
	bool IsHitWall(const Vector2& pos, float radius);
	// 落下判定
	bool IsFall(const Vector2& pos);
	// ゴール判定
	bool IsGoal(const Vector2& pos, float radius, bool canGoal);
	// 敵からの視認
	bool EnemyCanSeePlayer(const Vector3& enemyPos, const Vector3& playerPos);

	// 戦闘状態セット
	void SetCombat(bool isCombat);

private:	
	int WorldToMapX(float x) const;
	int WorldToMapY(float y) const;

	std::vector<std::vector<Tile>> map_;
	float tileSize_ = 0;
	bool isCombat_ = false;

	// バリアの出現タイマー
	Timer barrierAppierTimer_;
	Timer barrierDisappierTimer_;
	const float kBarrierAppierTime = 0.3f;
	const float kBarrierDisappierTime = 0.3f;
};

