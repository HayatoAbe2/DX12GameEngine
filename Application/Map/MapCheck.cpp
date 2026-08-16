#include "MapCheck.h"
#include "MapTile.h"
#include "Character/Player/Player.h"
#include <algorithm>

void MapCheck::Initialize(std::vector<std::vector<MapTile::Tile>> map, float tileSize) {
	map_ = map;
	tileSize_ = tileSize;
}

void MapCheck::Update(std::vector<std::vector<MapTile::Tile>> map) {
	map_ = map;
}

int MapCheck::WorldToMapX(float x) const {
	return static_cast<int>(std::floor(x / tileSize_ + 0.5f)) + static_cast<int>(map_[0].size()) / 2;
}

int MapCheck::WorldToMapY(float y) const {
	return static_cast<int>(std::floor(y / tileSize_ + 0.5f)) + static_cast<int>(map_.size()) / 2;
}

bool MapCheck::ResolveCollisionX(Vector2& pos, float radius, bool isFlying) {
	bool isHit = false;
	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());

	// キャラのAABB（更新後）
	float charMinX = pos.x - radius;
	float charMaxX = pos.x + radius;
	float charMinY = pos.y - radius;
	float charMaxY = pos.y + radius;

	// 衝突しそうな範囲だけループ（効率化）
	int startY = std::max(0, WorldToMapY(charMinY));
	int endY = std::min(mapH - 1, WorldToMapY(charMaxY));
	int startX = std::max(0, WorldToMapX(charMinX));
	int endX = std::min(mapW - 1, WorldToMapX(charMaxX));

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] == MapTile::Tile::Floor) continue;
			if (map_[y][x] == MapTile::Tile::None) {
				if (isFlying) { continue; }

				// 中心で判定
				charMinX = pos.x - 0.1f;
				charMaxX = pos.x + 0.1f;
				charMinY = pos.y - 0.1f;
				charMaxY = pos.y + 0.1f;
			}
			if (!isCombat_ && map_[y][x] == MapTile::CombatWall) continue;

			// タイルAABB
			float tileCenterX = (x - mapW / 2) * tileSize_;
			float tileCenterY = (y - mapH / 2) * tileSize_;

			float tileMinX = tileCenterX - tileSize_ * 0.5f;
			float tileMaxX = tileCenterX + tileSize_ * 0.5f;
			float tileMinY = tileCenterY - tileSize_ * 0.5f;
			float tileMaxY = tileCenterY + tileSize_ * 0.5f;

			// Y方向が重なってなければ無視（X軸解決なので）
			if (charMaxY <= tileMinY || charMinY >= tileMaxY) continue;

			// X方向重なり
			float overlapX = std::min(charMaxX, tileMaxX) - std::max(charMinX, tileMinX);
			if (overlapX > 0.0f) {
				float tileCenterX = (tileMinX + tileMaxX) * 0.5f;

				if (pos.x > tileCenterX)
					pos.x += (tileMaxX - charMinX); // 壁の右側にいる → 右に押す
				else
					pos.x -= (charMaxX - tileMinX); // 壁の左側にいる → 左に押す

				// 押し戻したのでAABB更新
				charMinX = pos.x - radius;
				charMaxX = pos.x + radius;

				isHit = true;
			}
		}
	}

	// --- マップ外チェック ---
	float minX = -mapW * tileSize_ * 0.5f;
	float maxX = mapW * tileSize_ * 0.5f;

	pos.x = std::clamp(pos.x, minX + radius, maxX - radius);

	return isHit;
}

bool MapCheck::ResolveCollisionY(Vector2& pos, float radius, bool isFlying) {
	bool isHit = false;
	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());

	float charMinX = pos.x - radius;
	float charMaxX = pos.x + radius;
	float charMinY = pos.y - radius;
	float charMaxY = pos.y + radius;

	int startY = std::max(0, WorldToMapY(charMinY));
	int endY = std::min(mapH - 1, WorldToMapY(charMaxY));
	int startX = std::max(0, WorldToMapX(charMinX));
	int endX = std::min(mapW - 1, WorldToMapX(charMaxX));

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] == MapTile::Tile::Floor) continue;
			if (map_[y][x] == MapTile::Tile::None) {
				if (isFlying) { continue; }

				// 中心で判定
				charMinX = pos.x - 0.1f;
				charMaxX = pos.x + 0.1f;
				charMinY = pos.y - 0.1f;
				charMaxY = pos.y + 0.1f;
			}
			if (!isCombat_ && map_[y][x] == MapTile::CombatWall) continue;

			float tileCenterX = (x - mapW / 2) * tileSize_;
			float tileCenterY = (y - mapH / 2) * tileSize_;

			float tileMinX = tileCenterX - tileSize_ * 0.5f;
			float tileMaxX = tileCenterX + tileSize_ * 0.5f;
			float tileMinY = tileCenterY - tileSize_ * 0.5f;
			float tileMaxY = tileCenterY + tileSize_ * 0.5f;

			// X方向が重なっていなければ無視
			if (charMaxX <= tileMinX || charMinX >= tileMaxX) continue;

			// Y方向重なり
			float overlapY = std::min(charMaxY, tileMaxY) - std::max(charMinY, tileMinY);
			if (overlapY > 0.0f) {
				float tileCenterY = (tileMinY + tileMaxY) * 0.5f;

				// 押し戻し方向
				if (pos.y > tileCenterY)
					pos.y += (tileMaxY - charMinY);
				else
					pos.y -= (charMaxY - tileMinY);

				// AABB更新
				charMinY = pos.y - radius;
				charMaxY = pos.y + radius;

				isHit = true;
			}
		}
	}

	// マップ外チェック
	float minY = -mapH * tileSize_ * 0.5f;
	float maxY = mapH * tileSize_ * 0.5f;

	pos.y = std::clamp(pos.y, minY + radius, maxY - radius);

	return isHit;
}

bool MapCheck::IsHitWall(const Vector2& pos, float radius) {
	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());

	// キャラのAABB（更新後）
	float charMinX = pos.x - radius;
	float charMaxX = pos.x + radius;
	float charMinY = pos.y - radius;
	float charMaxY = pos.y + radius;

	// 衝突しそうな範囲
	int startY = std::max(0, WorldToMapY(charMinY));
	int endY = std::min(mapH - 1, WorldToMapY(charMaxY));
	int startX = std::max(0, WorldToMapX(charMinX));
	int endX = std::min(mapW - 1, WorldToMapX(charMaxX));

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] == MapTile::Tile::Floor) continue;
			if (map_[y][x] == MapTile::Tile::None) continue;
			if (!isCombat_ && map_[y][x] == MapTile::CombatWall) continue;

			// タイルAABB
			float tileCenterX = (x - mapW / 2) * tileSize_;
			float tileCenterY = (y - mapH / 2) * tileSize_;

			float tileMinX = tileCenterX - tileSize_ * 0.5f;
			float tileMaxX = tileCenterX + tileSize_ * 0.5f;
			float tileMinY = tileCenterY - tileSize_ * 0.5f;
			float tileMaxY = tileCenterY + tileSize_ * 0.5f;

			// 重なっていれば衝突
			if (charMaxX > tileMinX || charMinX < tileMaxX &&
				charMaxY > tileMinY || charMinY < tileMaxY) {
				return true;
			}
		}
	}
	return false;
}

bool MapCheck::IsFall(const Vector2& pos) {
	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());

	// キャラのAABB（更新後）
	float charMinX = pos.x - 0.1f;
	float charMaxX = pos.x + 0.1f;
	float charMinY = pos.y - 0.1f;
	float charMaxY = pos.y + 0.1f;

	// 衝突しそうな範囲
	int startY = std::max(0, WorldToMapY(charMinY));
	int endY = std::min(mapH - 1, WorldToMapY(charMaxY));
	int startX = std::max(0, WorldToMapX(charMinX));
	int endX = std::min(mapW - 1, WorldToMapX(charMaxX));

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] != MapTile::Tile::None) {

				return false;
			}
		}
	}
	return true;
}

bool MapCheck::IsGoal(const Vector2& pos, float radius, bool canGoal) {
	if (!canGoal) { return false; }

	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());

	// キャラのAABB（更新後）
	float charMinX = pos.x - radius - 0.1f;
	float charMaxX = pos.x + radius;
	float charMinY = pos.y - radius - 0.1f;
	float charMaxY = pos.y + radius;

	// 衝突しそうな範囲
	int startY = std::max(0, WorldToMapY(charMinY));
	int endY = std::min(mapH - 1, WorldToMapY(charMaxY));
	int startX = std::max(0, WorldToMapX(charMinX));
	int endX = std::min(mapW - 1, WorldToMapX(charMaxX));

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] == MapTile::Tile::Goal) {

				// タイルAABB
				float tileCenterX = (x - mapW / 2) * tileSize_;
				float tileCenterY = (y - mapH / 2) * tileSize_;

				float tileMinX = tileCenterX - tileSize_ * 0.5f;
				float tileMaxX = tileCenterX + tileSize_ * 0.5f;
				float tileMinY = tileCenterY - tileSize_ * 0.5f;
				float tileMaxY = tileCenterY + tileSize_ * 0.5f;

				// 重なっていれば衝突
				if (charMaxX > tileMinX || charMinX < tileMaxX &&
					charMaxY > tileMinY || charMinY < tileMaxY) {
					return true;
				}
			}
		}
	}
	return false;
}

bool MapCheck::EnemyCanSeePlayer(const Vector3& enemyPos, const Vector3& playerPos) {
	Ray3D ray = { enemyPos, playerPos - enemyPos };
	ray.direction.y = 0.0001f;

	int mapH = static_cast<int>(map_.size());
	int mapW = static_cast<int>(map_[0].size());
	int startY = 0;
	int endY = mapH - 1;
	int startX = 0;
	int endX = mapW - 1;

	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			if (map_[y][x] == MapTile::Tile::Floor) { continue; }
			if (map_[y][x] == MapTile::Tile::None) { continue; }

			float tileCenterX = (x - mapW / 2) * tileSize_;
			float tileCenterZ = (y - mapH / 2) * tileSize_;

			AABB3D aabb;
			aabb.min = { tileCenterX - tileSize_ * 0.5f,0,tileCenterZ - tileSize_ * 0.5f };
			aabb.max = { tileCenterX + tileSize_ * 0.5f,0,tileCenterZ + tileSize_ * 0.5f };

			Vector3 tMin =
			{
				(aabb.min.x - ray.origin.x) / ray.direction.x,
				(aabb.min.y - ray.origin.y) / ray.direction.y,
				(aabb.min.z - ray.origin.z) / ray.direction.z
			};

			Vector3 tMax =
			{
				(aabb.max.x - ray.origin.x) / ray.direction.x,
				(aabb.max.y - ray.origin.y) / ray.direction.y,
				(aabb.max.z - ray.origin.z) / ray.direction.z
			};

			Vector3 tNear =
			{
				std::min(tMin.x , tMax.x),
				std::min(tMin.y , tMax.y),
				std::min(tMin.z , tMax.z)
			};

			Vector3 tFar =
			{
				std::max(tMin.x , tMax.x),
				std::max(tMin.y , tMax.y),
				std::max(tMin.z , tMax.z)
			};

			float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
			float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);

			if (tmax >= 0.0f && tmin <= tmax && tmin <= Length(ray.direction)) {
				return false;
			}
		}
	}
	return true;
}