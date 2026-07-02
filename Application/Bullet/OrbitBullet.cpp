#include "OrbitBullet.h"
#include "GameCommon.h"
#include "Map/MapCheck.h"

void OrbitBullet::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	particle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	particle_->SetLifeTime(2);
	particle_->SetColor(data_.bulletColor);

	hitParticle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	hitParticle_->SetLifeTime(hitParticleLifeTime);
	hitParticle_->SetColor(data_.bulletColor);
	particleField_ = std::make_unique<ParticleField>();
	particleField_->SetCheckArea(false);

	Vector3 dir = Normalize(velocity_);
	angle_ = std::atan2(dir.x, dir.z);
}

void OrbitBullet::Update(MapCheck* mapCheck, EffectManager* effectManager) {
	auto& ctx = GameContext::GetInstance();

	if (!isDead_) {
		prePos_ = model_->GetTransform().translate;
		model_->SetTranslate(user_->GetTransform().translate + 1.5f * Vector3(std::sin(angle_), 0.0f, std::cos(angle_)));
		angle_ += 0.06f;
		// マップ当たり判定
		Vector2 pos = { model_->GetTransform().translate.x,model_->GetTransform().translate.z };

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
			Transform transform = model_->GetTransform();
			transform.translate += randomVector + velocity_ * 0.5f;
			transform.scale = model_->GetTransform().scale * 2.0f;
			particle_->Emit(transform, -velocity_ * 0.5f);
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

void OrbitBullet::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	render.DrawParticle(particle_.get(), BlendMode::Add);
	render.DrawParticle(hitParticle_.get(), BlendMode::Add);
}

void OrbitBullet::Hit() {
	auto& ctx = GameContext::GetInstance();

	if (particleField_) {
		isDead_ = true;

		// 飛散パーティクル
		particleField_->SetGravity(-0.4f, model_->GetTransform().translate);
		hitParticle_->AddField(std::move(particleField_));
		for (int i = 0; i < hitParticleNum_; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform = model_->GetTransform();
			transform.translate += randomVector;
			transform.scale = model_->GetTransform().scale * 3.0f;
			hitParticle_->Emit(transform, {});
		}
	}
}