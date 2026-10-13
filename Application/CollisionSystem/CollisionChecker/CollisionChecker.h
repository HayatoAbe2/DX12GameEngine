#pragma once
#include "Character/Character.h"

class Player;
class Enemy;
class Bullet;
class Camera;
class EffectManager;
class BulletManager;
class EnemyManager;
class MapCheck;

class CollisionChecker {
public:
	void Initialize(EffectManager* effectManager, Camera* camera);

	// プレイヤーと敵弾
	void CheckPlayer(Player* player, Bullet* bullet);

	// 敵と自弾
	void CheckEnemy(Enemy* enemy, Bullet* bullet, Player* player);

	// 接触ダメージ確認
	void CheckContactDamage(Player* player, Enemy* enemy);

	void CheckPush(Character* characterA, Character* characterB, MapCheck* mapCheck);

	// 押し出し処理

private:
	EffectManager* effectManager_;
	Camera* camera_;
};

