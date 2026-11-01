#pragma once
#include "Objects/Character/Character.h"

class Player;
class Enemy;
class Bullet;
class Camera;
class EffectManager;
class BulletManager;
class DamageAreaManager;
class DamageArea;
class EnemyManager;
class MapCheck;

/// <summary>
/// 詳細な衝突判定と衝突時処理
/// </summary>
class CollisionChecker {
public:
	CollisionChecker(EffectManager* effectManager, DamageAreaManager* damageAreaManager, Camera* camera) :
		effectManager_(effectManager), damageAreaManager_(damageAreaManager), camera_(camera) {
	}

	// プレイヤーと敵弾
	void CheckPlayer(Player* player, Bullet* bullet);
	// プレイヤーとエリア
	void CheckPlayer(Player* player, DamageArea* area);

	// 敵と自弾
	void CheckEnemy(Enemy* enemy, Bullet* bullet, Player* player);
	// 敵とエリア
	void CheckEnemy(Enemy* enemy, DamageArea* area, Player* player);

	// 接触ダメージ確認
	void CheckContactDamage(Player* player, Enemy* enemy);

	// 押し出し処理
	void CheckPush(Character* characterA, Character* characterB, MapCheck* mapCheck);

private:
	EffectManager* effectManager_;
	DamageAreaManager* damageAreaManager_;
	Camera* camera_;

	// 重なり量に対して押し出す大きさ
	const float kPushRate = 0.2f;

	// 最大移動距離(元の位置から移動しすぎるのを抑制)
	float maxDistance = 0.25f;
};

