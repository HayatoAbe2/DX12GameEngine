#pragma once
#include "Bullet.h"
#include "GameCommon.h"

class MapCheck;
class EffectManager;

class BulletManager {
public:
	void Update(MapCheck* mapCheck);
	void Draw(Camera* camera);
	void Reset();

	void AddBullet(std::unique_ptr<Bullet> newBullet);
	std::vector<Bullet*> GetBullets();
private:
	// 弾
	std::vector<std::unique_ptr<Bullet>> bullets_;
};

