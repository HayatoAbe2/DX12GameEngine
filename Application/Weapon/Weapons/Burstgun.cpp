#include "Burstgun.h"
#include "Bullet/BulletManager.h"
#include "Bullet/SpreadBullet.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>

float Burstgun::Trigger(Vector3 pos, Vector3 dir, BulletManager* bulletManager, Camera* camera, Character* from) {
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;
	}

	bulletManager_ = bulletManager;
	from_ = from;
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
	bullet->SetTranslate(pos);
	std::unique_ptr<SpreadBullet> newBullet = std::make_unique<SpreadBullet>(std::move(bullet), dir, data_, from);
	newBullet->Initialize();
	burstTimer_[0]->Start(0.1f);
	burstTimer_[1]->Start(0.2f);

	for (int i = 0; i < 2; ++i) {
		burst_[i] = false;
	}

	bulletManager->AddBullet(std::move(newBullet));

	audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);
		SetChargeStartTimer();

		return data_.stats.shootCoolTime;
	}
}

void Burstgun::Update() {
	Weapon::Update();

	for (int i = 0; i < 2; ++i) {
		if (burstTimer_[i]->IsActive()) {
			burstTimer_[i]->Update();

			if (burstTimer_[i]->IsFinished() && !burst_[i]) {
				burst_[i] = true;

				auto& ctx = GameContext::GetInstance();
				auto& asset = ctx.Asset();
				auto& audio = ctx.Audio();
				auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");
				bullet->SetTranslate(from_->GetTransform().translate);
				Vector3 dir = Vector3(std::sin(model_->GetTransform().rotate.y), 0.0f, std::cos(model_->GetTransform().rotate.y));
				std::unique_ptr<SpreadBullet> newBullet = std::make_unique<SpreadBullet>(std::move(bullet), dir, data_, from_);
				newBullet->Initialize();
				bulletManager_->AddBullet(std::move(newBullet));
				audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);
			}
		}
	}
}

