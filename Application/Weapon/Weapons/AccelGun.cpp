#include "AccelGun.h"
#include "Bullet/BulletManager.h"
#include "Bullet/AccelBullet.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>

float AccelGun::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {	
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;
	}
	
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<AccelBullet> newBullet = std::make_unique<AccelBullet>(std::move(bullet), dir, data_, from);
	newBullet->Initialize();

	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);

		camera->StartShake(0.2f, 2);
		return data_.stats.shootCoolTime;
	}
}

void AccelGun::Update() {
	float deltatime = GameContext::GetInstance().GetDeltatime();
	charge_ = (std::min)(data_.stats.maxCharge, charge_ + data_.stats.chargeTime * deltatime);
}

