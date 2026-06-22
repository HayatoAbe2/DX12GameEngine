#include "AssaultRifle.h"
#include "Bullet/BulletManager.h"
#include "Bullet/NormalBullet.h"
#include <Character/Enemy/Enemy.h>

float AssaultRifle::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<NormalBullet> newBullet = std::make_unique<NormalBullet>(std::move(bullet), dir, data_, from);
	newBullet->Initialize();
	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		camera->StartShake(0.5f, 2);
		return data_.stats.shootCoolTime;
	}
}

void AssaultRifle::Update() {
}

