#include "AssaultRifle.h"
#include "Bullet/BulletManager.h"
#include "Bullet/NormalBullet.h"

AssaultRifle::AssaultRifle(const WeaponStatus& status, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	status_ = status;
	ammoLeft_ = status.magazineSize;
	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel);
}

int AssaultRifle::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, bool isEnemyBullet) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<NormalBullet> newBullet = std::make_unique<NormalBullet>(std::move(bullet), dir, status_, isEnemyBullet);
	newBullet->Initialize();
	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (isEnemyBullet) {
		return status_.shootCoolTime * 2;
	} else {
		camera->StartShake(0.5f, 2);
		ammoLeft_--;
		return status_.shootCoolTime;
	}
}

void AssaultRifle::Update() {
	if (isReloading_) {
		reloadTimer_--;

		if (reloadTimer_ <= 0) {
			ammoLeft_ = status_.magazineSize;
			isReloading_ = false;
		}
	}
}

