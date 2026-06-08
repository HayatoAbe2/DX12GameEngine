#include "Bullet.h"
#include "GameCommon.h"
#include <numbers>

Bullet::Bullet(std::unique_ptr<Model> model, const Vector3& direction, const WeaponData& data, bool isEnemyBullet) {
	model_ = std::move(model);
	data_ = data;

	velocity_ = direction * data.bulletSpeed;
	if (isEnemyBullet) { velocity_ /= 2.0f; }
	lifeTime_ = data.bulletLifeTime;
	isEnemyBullet_ = isEnemyBullet;
	model_->SetScale({ data.bulletSize,data.bulletSize ,data.bulletSize });
	model_->SetRotate({ 0, -std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f,0 });
}