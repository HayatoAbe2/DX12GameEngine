#include "Shotgun.h"
#include "Bullet/BulletManager.h"
#include "Bullet/SpreadBullet.h"
#include <numbers>

Shotgun::Shotgun(const WeaponStatus& status, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	status_ = status;
	ammoLeft_ = status.magazineSize;
	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel);
}

int Shotgun::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, bool isEnemyBullet) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	for (int i = -2; i <= 2; ++i) {
		auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
		bullet->SetTranslate(pos);

		Vector3 rotatedDir = TransformVector(dir, MakeRotateYMatrix(float(std::numbers::pi) / 16.0f * i));
		std::unique_ptr<SpreadBullet> newBullet = std::make_unique<SpreadBullet>(std::move(bullet), rotatedDir, status_, isEnemyBullet);
		newBullet->Initialize();

		bulletManager->AddBullet(std::move(newBullet));
	}
	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (isEnemyBullet) {
		return status_.shootCoolTime * 2;
	} else {
		camera->StartShake(1.5f, 5);
		ammoLeft_--;
		return status_.shootCoolTime;
	}
}

void Shotgun::Update() {
}

