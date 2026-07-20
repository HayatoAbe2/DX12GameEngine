#include "ChargeGun.h"
#include "Bullet/BulletManager.h"
#include "Bullet/SpreadBullet.h"
#include "Character/Enemy/Enemy.h"
#include <Character/Player/Player.h>

float ChargeGun::Trigger(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	auto data = data_;
	data.stats.bulletSize += charge_;
	data.stats.damage += charge_ * 3.0f;
	std::unique_ptr<SpreadBullet> newBullet = std::make_unique<SpreadBullet>(std::move(bullet), dir, data, from);
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

void ChargeGun::Update() {
	Weapon::Update();

	auto mat = model_->GetMaterial(1)->GetData();
	mat.color = Vector4(charge_,0,0,1);
	model_->GetMaterial(1)->SetData(mat);
}

