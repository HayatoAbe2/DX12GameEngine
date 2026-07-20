#include "Bullet.h"
#include "GameCommon.h"
#include "Character/Enemy/Enemy.h"
#include "Map/MapCheck.h"
#include <numbers>

Bullet::Bullet(const Vector2& direction, const BulletData& data, Character* from) {
	data_ = data;
	user_ = from;

	collider_.center = from->GetTransform().translate;
	collider_.radius = data_.radius;

	// 敵弾の色統一
	if (dynamic_cast<Enemy*>(user_)) {
		data_.speed *= 0.5f;
		data_.lifeTime *= 5;
		data_.color = { 0.8f,0,0,1.0f };
	}

	auto& ctx = GameContext::GetInstance();
	velocity_ = direction * data_.speed;

	if (dynamic_cast<Enemy*>(from)) {
		isEnemyBullet_ = true;
		velocity_ /= 2.0f;
	}
	lifeTime_ = data_.lifeTime;
}

void Bullet::Update(MapCheck* mapCheck, EffectManager* effectManager) {
	auto& ctx = GameContext::GetInstance();

	Move(mapCheck, effectManager);

	if (!isDead_) {
		lifeTime_--;
		if (lifeTime_ <= 0) {
			Hit();
		}

		// パーティクル
		for (int i = 0; i < 50; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform = {
				{data_.radius,data_.radius,data_.radius},
				{0,0,0},
				{collider_.center.x, data_.radius, collider_.center.y} }; 
			particle_->Emit(transform, -Vector3{ velocity_.x, 0, velocity_.y } * 0.5f);
		}
	}
	particle_->Update();

	if (isDead_) {
		hitParticle_->Update();
		hitParticleLifeTime--;
		if (hitParticleLifeTime <= 0) {
			canErase_ = true;
		}
	}
}

void Bullet::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	render.DrawParticle(particle_.get(), BlendMode::Add);
	render.DrawParticle(hitParticle_.get(), BlendMode::Add);
}

void Bullet::Hit() {
	auto& ctx = GameContext::GetInstance();

	if (particleField_) {
		isDead_ = true;

		// 飛散パーティクル
		particleField_->SetGravity(-0.4f, {collider_.center.x, 0.5f, collider_.center.y});
		hitParticle_->AddField(std::move(particleField_));
		for (int i = 0; i < hitParticleNum_; ++i) {
			Transform transform = { collider_.center.x, 0.5f, collider_.center.y };
			transform.scale = Vector3{ 0.05f, 1.0f, 1.0f } * 5;
			transform.rotate = { 0,0,ctx.RandomFloat(0, float(std::numbers::pi) * 2.0f) };
			hitParticle_->Emit(transform, {});
		}
	}
}

void Bullet::Move(MapCheck* mapCheck, EffectManager* effectManager) {
	auto& ctx = GameContext::GetInstance();
	prePos_ = collider_.center;

	if (data_.traits.move.orbit) {
		float angle = data_.traits.move.orbit->angle;
		float speed = data_.traits.move.orbit->speed;
		float radius = data_.traits.move.orbit->radius;
		collider_.center = ToXZ(user_->GetTransform().translate) + radius * Vector2(std::sin(angle), std::cos(angle));
		data_.traits.move.orbit->angle += speed;
	}

	if (data_.traits.move.wave) {
		Vector2 currentVel = velocity_;cv   
		float sinWave_ = sinf(0.4f * float(std::numbers::pi) * data_.traits.move.wave->time - 0.5f);
		currentVel = TransformVector(velocity_, MakeRotateYMatrix(float(std::numbers::pi) / 5.0f * sinWave_)); 
	}

	// 加速
	if (data_.traits.move.accel) {
		Vector2 dir = Normalize(velocity_);
		float length = Length(velocity_);
		velocity_ = dir * (length * data_.traits.move.accel->amount);
	}

	// 反射する場合、XとZ軸を片方ずつ動かす
	if (data_.traits.onHitWall.ricochet) {
		collider_.center.x += velocity_.x * ctx.GetDeltatime();
		// 反射
		if (mapCheck->IsHitWall(collider_.center, collider_.radius)) {
			if (isEnemyBullet_) Hit();

			data_.traits.onHitWall.ricochet->current++;
			velocity_.x *= -1;
			collider_.center.x += velocity_.x * 2 * ctx.GetDeltatime();
		}

		collider_.center.y += velocity_.y * ctx.GetDeltatime();
		// 反射
		if (mapCheck->IsHitWall(collider_.center, collider_.radius)) {
			if (isEnemyBullet_) Hit();

			data_.traits.onHitWall.ricochet->current++;
			velocity_.y *= -1;
			collider_.center.y += velocity_.y * 2 * ctx.GetDeltatime();
		}
	} else {
		if (mapCheck->IsHitWall(collider_.center, collider_.radius)) {
			OnHitWall(effectManager);
		}

	}
}

void Bullet::OnHitWall(EffectManager* effectManager) {
	Hit();
	if (data_.traits.onHitAnything.explode) {
		effectManager->SpawnExplodeEffect({ collider_.center.x, 0.5f, collider_.center.y });
	}
}

void Bullet::OnHitAnything(EffectManager* effectManager) {
	Hit();
	if (data_.traits.onHitAnything.explode) {
		effectManager->SpawnExplodeEffect({ collider_.center.x, 0.5f, collider_.center.y });
	}
}
