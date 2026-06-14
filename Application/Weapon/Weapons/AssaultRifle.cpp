#include "AssaultRifle.h"
#include "Bullet/BulletManager.h"
#include "Bullet/NormalBullet.h"

float AssaultRifle::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, bool isEnemyBullet) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<NormalBullet> newBullet = std::make_unique<NormalBullet>(std::move(bullet), dir, data_, isEnemyBullet);
	newBullet->Initialize();
	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (isEnemyBullet) {
		return data_.stats.shootCoolTime * 2;
	} else {
		camera->StartShake(0.5f, 2);
		return data_.stats.shootCoolTime;
	}
}

void AssaultRifle::Update() {
}

