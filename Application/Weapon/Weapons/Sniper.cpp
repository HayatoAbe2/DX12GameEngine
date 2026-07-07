#include "Sniper.h"
#include "Bullet/BulletManager.h"
#include "Bullet/RapidBullet.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>

float Sniper::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;

	}
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<RapidBullet> newBullet = std::make_unique<RapidBullet>(std::move(bullet), dir, data_, from);
	newBullet->Initialize();

	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);
		SetChargeStartTimer();

		camera->StartShake(0.2f, 2);
		return data_.stats.shootCoolTime;
	}
}

void Sniper::Update() {
	Weapon::Update();
}

