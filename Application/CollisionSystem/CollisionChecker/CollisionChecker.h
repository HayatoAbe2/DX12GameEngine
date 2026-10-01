#pragma once

class Player;
class Enemy;
class Bullet;
class Camera;
class EffectManager;
class BulletManager;
class EnemyManager;

class CollisionChecker {
public:
	void Initialize(EffectManager* effectManager, Camera* camera);

	// プレイヤーと敵弾
	void CheckPlayer(Player* player, Bullet* bullet);

	// 敵と自弾
	void CheckEnemy(Enemy* enemy, Bullet* bullet, Player* player);

	// 接触ダメージ確認
	void CheckContact(Player* player, Enemy* enemy);

private:
	EffectManager* effectManager_;
	Camera* camera_;
};

