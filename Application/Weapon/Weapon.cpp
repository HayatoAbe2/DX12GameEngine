#include "Weapon.h"
#include <Character/Enemy/Enemy.h>
#include <Character/Player/Player.h>
#include <CollisionChecker/CollisionChecker.h>

Weapon::Weapon(const WeaponData& data, std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel) {
	data_ = data;
	charge_ = data_.stats.maxCharge;

	model_ = std::move(model);
	shadowModel_ = std::move(shadowModel);
}

float Weapon::Trigger(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from) {
	// 撃てない
	if (dynamic_cast<Player*>(from) && charge_ < 1.0f) {
		return 0;
	}

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 溜め(離して発射)
	if (data_.traits.charge) {
		data_.traits.charge->currentTime += ctx.GetDeltatime();
		data_.traits.charge->currentTime = (std::min)(data_.traits.charge->currentTime, data_.traits.charge->time);
		data_.traits.charge->charging = true;
		canChange_ = false;
		return 0;
	}

	return Fire(pos, dir, bulletManager, from);
}

void Weapon::Update(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from) {
	reloadStartTimer_.Update();
	if (reloadStartTimer_.IsFinished()) {
		float deltatime = GameContext::GetInstance().GetDeltatime();
		float chargeTime = data_.stats.chargeTime * data_.modifiers[int(ModifierStats::coolTime)].multiply;
		charge_ = (std::min)(data_.stats.maxCharge, charge_ + deltatime / chargeTime);
	}

	if (data_.traits.burst) {
		auto& current = data_.traits.burst->currentCount;
		auto& maxCount = data_.traits.burst->count;
		auto& timer = data_.traits.burst->timer;
		timer.Update();

		if (timer.IsFinished() && current < maxCount) {
			// 方向は一度目のもので固定
			Fire(pos, data_.traits.burst->dir, bulletManager, from);
		}
	}

	if (data_.traits.charge) {
		// 長押しをやめたら発射
		if (!data_.traits.charge->charging && data_.traits.charge->time > 0) {
			Fire(pos, dir, bulletManager, from);
			canChange_ = true;
		}
	}
}

float Weapon::Fire(const Vector3& pos, const Vector2& dir, BulletManager* bulletManager, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	// バースト射撃
	if (data_.traits.burst) {
		auto& current = data_.traits.burst->currentCount;
		auto& maxCount = data_.traits.burst->count;
		current++;
		data_.traits.burst->dir = dir;

		if (current < maxCount) {
			// 追加射撃の予約
			data_.traits.burst->timer.Start(data_.traits.burst->interval);
			canChange_ = false;
		} else {
			// 終了
			current = 0;
			canChange_ = true;
		}
	}

	// 複数弾
	if (data_.traits.multiShot && data_.traits.multiShot->count > 1) {
		int count = data_.traits.multiShot->count;
		// 度からラジアンに変換
		float maxAngle = data_.traits.multiShot->maxAngle * float(std::numbers::pi) / 180.0f;

		if (!data_.traits.multiShot->random) {
			// 角度の間隔
			float step = maxAngle / float(count - 1);

			for (int i = 0; i <= count; ++i) {
				float angle = -maxAngle * 0.5f + step * i;
				float cos = std::cos(angle);
				float sin = std::sin(angle);
				Vector2 rotatedDir{
					dir.x * cos - dir.y * sin,
					dir.y * sin + dir.y * cos
				};

				// 弾生成
				std::unique_ptr<Bullet> bullet = std::make_unique<Bullet>(ToXZ(pos), rotatedDir, data_.bullet, from);
				// BulletManagerの管理下に移動
				bulletManager->AddBullet(std::move(bullet));
			}
		}

	} else {

		// 弾生成
		std::unique_ptr<Bullet> bullet = std::make_unique<Bullet>(ToXZ(pos), dir, data_.bullet, from);
		// BulletManagerの管理下に移動
		bulletManager->AddBullet(std::move(bullet));

		// 効果音
		audio.SoundPlay(L"Resources/Sounds/SE/shoot.mp3", false);
	}

	if (dynamic_cast<Enemy*>(from)) {
		return data_.stats.shootCoolTime * 2;
	} else {
		charge_--;
		charge_ = (std::max)(charge_, 0.0f);
		SetChargeStartTimer();

		// カメラシェイク
		ctx.Render().GetCamera()->StartShake(0.2f, 2);

		return data_.stats.shootCoolTime;
	}
}

void Weapon::SetChargeStartTimer() {
	// 回復開始までの時間
	reloadStartTimer_.Start(0.75f);
}
