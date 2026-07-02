#include "ChargeGun.h"
#include "Bullet/BulletManager.h"
#include "Bullet/SpreadBullet.h"
#include "Character/Enemy/Enemy.h"

float ChargeGun::Shoot(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	auto data = data_;
	data.stats.bulletSize += (damageBonus_ / 3.0f);
	data.stats.damage += damageBonus_;
	std::unique_ptr<SpreadBullet> newBullet = std::make_unique<SpreadBullet>(std::move(bullet), dir, data, from);
	newBullet->Initialize();
	damageBonus_ = 0;

	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		camera->StartShake(0.2f, 2);
		return data_.stats.shootCoolTime;
	}
}

void ChargeGun::Update() {
	damageBonus_ = (std::min)(3.0f, damageBonus_ += 0.02f);
	auto mat = model_->GetMaterial(1)->GetData();
	mat.color = Vector4(damageBonus_ / 3.0f,0,0,1);
	model_->GetMaterial(1)->SetData(mat);
}

