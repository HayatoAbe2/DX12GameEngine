#pragma once
#include "Bullet.h"
#include "GameCommon.h"
class FireBullet : public Bullet {
public:
	FireBullet(std::unique_ptr<Model> model, const Vector3& direction, const WeaponData& data, bool isEnemyBullet) :
		Bullet(std::move(model), direction, data, isEnemyBullet) {
	};
	void Initialize();
	void Update(MapCheck* mapCheck, EffectManager* effectManager) override;
	void Draw(Camera* camera) override;
	void Hit() override;

private:
	std::unique_ptr<ParticleSystem> particle_;
	const int particleNum_ = 300;
	float particleRange_ = 1.5f;

	std::unique_ptr<ParticleSystem> explosionParticle_;
	const int explosionParticleNum_ = 250;
	float explosionParticleRange_ = 1.0f;
	int explosionEndLifeTime = 8;

	std::unique_ptr<ParticleField> particleField_;

	// ライトのインデックス
	int lightIndex_ = 0;

	// カメラ揺らし
	bool shaked_ = false;
};

