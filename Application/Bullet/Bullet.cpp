#include "Bullet.h"
#include "GameCommon.h"
#include "Character/Enemy/Enemy.h"
#include "Map/MapCheck.h"
#include <numbers>

Bullet::Bullet(const Vector2& pos, const Vector2& direction, const BulletData& data, Character* from) {
	data_ = data;
	user_ = from;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	collider_.center = pos;
	collider_.radius = data_.radius;

	if (data_.traits.move.orbit) {
		Vector2 offset = direction;

		// angleを生成位置から決める
		data_.traits.move.orbit->angle = std::atan2(offset.y, offset.x);
	}

	// 敵弾の色統一
	if (dynamic_cast<Enemy*>(user_)) {
		data_.speed *= 0.5f;
		data_.lifeTime *= 5;
		data_.color = { 0.7f, 0.2f, 0.1f, data_.color.w };
	}

	velocity_ = direction * data_.speed;

	if (dynamic_cast<Enemy*>(from)) {
		isEnemyBullet_ = true;
		velocity_ /= 2.0f;
	}
	lifeTime_ = data_.lifeTime;

	// パーティクル
	particle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	particle_->SetLifeTime(2);
	particle_->SetColor(data_.color);

	hitParticle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	hitParticle_->SetLifeTime(hitParticleLifeTime);
	hitParticle_->SetColor(data_.color);
	particleField_ = std::make_unique<ParticleField>();
	particleField_->SetCheckArea(false);
}

void Bullet::Update(MapCheck* mapCheck, EffectManager* effectManager) {
	auto& ctx = GameContext::GetInstance();

	if (!isDead_) {
		Move(mapCheck, effectManager);

		lifeTime_--;
		if (lifeTime_ <= 0) {
			isDead_ = true;
		}
	}

	if (isDead_) {
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
	if (!isDead_) {
		for (int i = 0; i < 50; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform = {
				Vector3{data_.radius,data_.radius,data_.radius} * 2.0f,
				{0,0,0},
				Vector3{collider_.center.x, data_.radius, collider_.center.y} + randomVector };
			particle_->Emit(transform, -Vector3{ velocity_.x, 0, velocity_.y } * 0.5f * ctx.GetDeltatime());
		}
	}
	particle_->Update();
	hitParticle_->Update();

	// パーティクル
	render.DrawParticle(particle_.get(), BlendMode::Add);
	render.DrawParticle(hitParticle_.get(), BlendMode::Add);
}

void Bullet::Hit() {
	auto& ctx = GameContext::GetInstance();

	if (particleField_) {
		if (data_.traits.onHitEnemy.piercing) {
			int& current = data_.traits.onHitEnemy.piercing->current;
			int& maxCount = data_.traits.onHitEnemy.piercing->count;

			if (current < maxCount) {
				// 貫通(消さない)
				current++;
			} else {
				isDead_ = true;
			}

		} else {
			isDead_ = true;
		}

		// 飛散パーティクル
		particleField_->SetGravity(-0.4f, { collider_.center.x, 0.5f, collider_.center.y });
		hitParticle_->AddField(std::move(particleField_));
		for (int i = 0; i < hitParticleNum_; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform;
			transform.translate = Vector3(collider_.center.x, 0.5f, collider_.center.y) + randomVector;
			transform.rotate = { 0,0,ctx.RandomFloat(0, float(std::numbers::pi) * 2.0f) };
			transform.scale = Vector3(collider_.radius, collider_.radius, collider_.radius) * 3.0f;
			hitParticle_->Emit(transform, {});
		}
	} else {
		if (data_.traits.onHitEnemy.piercing) {
			int& current = data_.traits.onHitEnemy.piercing->current;
			int& maxCount = data_.traits.onHitEnemy.piercing->count;

			if (current < maxCount) {
				// 貫通(消さない)
				current++;
			} else {
				isDead_ = true;
			}

		}
	}
}

void Bullet::Move(MapCheck* mapCheck, EffectManager* effectManager) {
	auto& ctx = GameContext::GetInstance();
	prePos_ = collider_.center;

	// 軌道回転
	if (data_.traits.move.orbit) {
		float angle = data_.traits.move.orbit->angle;
		float speed = data_.traits.move.orbit->speed;
		float radius = data_.traits.move.orbit->radius;
		collider_.center = ToXZ(user_->GetTransform().translate) + radius * Vector2(std::sin(angle), std::cos(angle));
		data_.traits.move.orbit->angle += speed * ctx.GetDeltatime();

		return;
	}

	// 波
	if (data_.traits.move.wave) {
		Vector2 currentVel = velocity_;
		float amplitude = data_.traits.move.wave->amplitude;
		float speed = data_.traits.move.wave->speed;
		float maxAngle = data_.traits.move.wave->maxAngle * float(std::numbers::pi) / 180.0f;
		float sinWave_ = sinf(speed * float(std::numbers::pi) * data_.traits.move.wave->time) * amplitude;
		float angle = std::sin(data_.traits.move.wave->time * data_.traits.move.wave->speed * 2.0f * float(std::numbers::pi)) * maxAngle;
		currentVel = ToXZ(TransformVector(Vector3(velocity_.x, 0, velocity_.y), MakeRotateYMatrix(angle)));
		collider_.center += currentVel * ctx.GetDeltatime();
		data_.traits.move.wave->time += ctx.GetDeltatime();

		if (mapCheck->IsHitWall(collider_.center, collider_.radius)) {
			OnHitWall(effectManager);
		}
		return;
	}

	// 加速
	if (data_.traits.move.accel) {
		velocity_ *= data_.traits.move.accel->rate;
	}

	// 反射する場合、XとZ軸を片方ずつ動かす
	if (data_.traits.onHitWall.ricochet && data_.traits.onHitWall.ricochet->current < data_.traits.onHitWall.ricochet->count) {
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
		collider_.center += velocity_ * ctx.GetDeltatime();
		if (mapCheck->IsHitWall(collider_.center, collider_.radius)) {
			OnHitWall(effectManager);
		}

	}
}

void Bullet::OnHitWall(EffectManager* effectManager) {
	Hit();

	// 仮処理(貫通が壁で消えないため)
	if (data_.traits.onHitEnemy.piercing) {
		isDead_ = true;
	}

	OnHitAnything(effectManager);
}

void Bullet::OnHitAnything(EffectManager* effectManager) {
	if (data_.traits.onHitAnything.explode) {
		effectManager->SpawnExplodeEffect({ collider_.center.x, 0.5f, collider_.center.y });
	}
}
