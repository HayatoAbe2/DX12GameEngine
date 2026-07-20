#include "Pistol.h"
#include "Bullet/BulletManager.h"
#include "Bullet/NormalBullet.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>

float Pistol::Trigger(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;
	}

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
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);
		SetChargeStartTimer();

		camera->StartShake(0.2f, 2);
		return data_.stats.shootCoolTime;
	}
}

void Pistol::Update() {
	Weapon::Update();
}

