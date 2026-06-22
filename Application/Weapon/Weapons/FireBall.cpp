#include "FireBall.h"
#include "Bullet/BulletManager.h"
#include "Bullet/FireBullet.h"
#include <Character/Enemy/Enemy.h>

float FireBall::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<FireBullet> newBullet = std::make_unique<FireBullet>(std::move(bullet), dir, data_, from);
	newBullet->Initialize();

	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/fire.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		return data_.stats.shootCoolTime;
	}
}

void FireBall::Update() {
}

