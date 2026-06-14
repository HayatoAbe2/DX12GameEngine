#include "Bullet.h"
#include "GameCommon.h"
#include <numbers>

Bullet::Bullet(std::unique_ptr<Model> model, const Vector3& direction, const WeaponData& data, bool isEnemyBullet) {
	model_ = std::move(model);
	data_ = data;

	auto& ctx = GameContext::GetInstance();
	velocity_ = direction * data.stats.bulletSpeed * ctx.GetDeltatime();

	if (isEnemyBullet) { velocity_ /= 2.0f; }
	lifeTime_ = data.stats.bulletLifeTime;
	isEnemyBullet_ = isEnemyBullet;

	model_->SetScale({ data.stats.bulletSize,data.stats.bulletSize ,data.stats.bulletSize });
	model_->SetRotate({ 0, -std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f,0 });
}