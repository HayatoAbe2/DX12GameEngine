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
	void Initialize(EffectManager* effectManager);
	void Check(Player* player, Bullet* bullet, Camera* camera, BulletManager* bulletManager);
	void Check(Enemy* enemy, Bullet* bullet, Camera* camera, Player* player, EnemyManager* enemyManager);
	void Check(Player* player, Enemy* enemy, Camera* camera);

private:
	EffectManager* effectManager_;
};

