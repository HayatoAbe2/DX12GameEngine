#pragma once
#include "Bullet.h"
#include "GameCommon.h"

class NormalBullet :public Bullet{
public:
	NormalBullet(std::unique_ptr<Model> model, const Vector3& direction, const WeaponStatus& status, bool isEnemyBullet) :
		Bullet(std::move(model), direction, status, isEnemyBullet) {
	};
	void Initialize();
	void Update(MapCheck* mapCheck) override;
	void Draw(Camera* camera) override;
	void Hit() override;

private:
	std::unique_ptr<ParticleSystem> particle_;
	const int particleNum_ = 150;
	float particleRange_ = 0.7f;

	std::unique_ptr<ParticleSystem> hitParticle_;
	const int hitParticleNum_ = 10;
	float hitParticleRange_ = 1.0f;
	int hitParticleLifeTime = 20;

	std::unique_ptr<ParticleField> particleField_;
};

