#include "Weapon.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>

Weapon::Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	data_ = data;
	charge_ = data_.stats.maxCharge;

	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel); 
	reloadStartTimer_ = std::make_unique<Timer>();
}

float Weapon::Trigger(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	// 撃てない
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;
	}

	return Fire(pos, dir, bulletManager, camera, from);
}

void Weapon::Update() {
	reloadStartTimer_->Update();
	if (reloadStartTimer_->IsFinished()) {
		float deltatime = GameContext::GetInstance().GetDeltatime();
		charge_ = (std::min)(data_.stats.maxCharge, charge_ + deltatime / data_.stats.chargeTime);
	}
}

float Weapon::Fire(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	// 弾生成
	std::unique_ptr<Bullet> bullet = std::make_unique<Bullet>(dir, data_.bullet, from);
	bullet->Initialize();

	// BulletManagerの管理下に移動
	bulletManager->AddBullet(std::move(bullet));

	// 効果音
	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);
		SetChargeStartTimer();

		// カメラシェイク
		camera->StartShake(0.2f, 2);

		return data_.stats.shootCoolTime;
	}
}

void Weapon::SetChargeStartTimer() {
	// 回復開始までの時間
	reloadStartTimer_->Start(0.5f);
}
