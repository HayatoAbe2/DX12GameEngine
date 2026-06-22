#include "Bullet.h"
#include "GameCommon.h"
#include <numbers>
#include <Character/Enemy/Enemy.h>

Bullet::Bullet(std::unique_ptr<Model> model, const Vector3& direction, const WeaponData& data, Character* from) {
	model_ = std::move(model);
	data_ = data;
	user_ = from;

	auto& ctx = GameContext::GetInstance();
	velocity_ = direction * data.stats.bulletSpeed * ctx.GetDeltatime();

	if (dynamic_cast<Enemy*>(from)) {
		isEnemyBullet_ = true; 
		velocity_ /= 2.0f; 
	}
	lifeTime_ = data.stats.bulletLifeTime;

	model_->SetScale({ data.stats.bulletSize,data.stats.bulletSize ,data.stats.bulletSize });
	model_->SetRotate({ 0, -std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f,0 });

	noHitTimer_ = std::make_unique<Timer>();
}
