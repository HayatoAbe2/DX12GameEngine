#include "RapidBullet.h"
#include "Map/MapCheck.h"

void RapidBullet::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	particle_ = asset.CreateParticleSystem(ParticleShape::Plane, asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	particle_->SetLifeTime(2);
	particle_->SetColor({ 0.0f, 0.1f, 0.8f, 1.0f });

	hitParticle_ = asset.CreateParticleSystem(ParticleShape::Plane, asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), particleNum_);
	hitParticle_->SetLifeTime(hitParticleLifeTime);
	hitParticle_->SetColor({ 0.2f, 0.1f, 1.0f, 1.0f });
	particleField_ = std::make_unique<ParticleField>();
	particleField_->SetCheckArea(false);
}

void RapidBullet::Update(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();

	if (!isDead_) {
		prePos_ = model_->GetTransform().translate;
		model_->SetTranslate(model_->GetTransform().translate + velocity_);

		// マップ当たり判定
		Vector2 pos = { model_->GetTransform().translate.x,model_->GetTransform().translate.z };

		lifeTime_--;
		if (lifeTime_ <= 0) {
			Hit();
		}

		if (mapCheck->IsHitWall(pos, data_.stats.bulletSize / 2.0f)) {
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
	hitParticle_->Update();
	noHitTimer_->Update();

	if (isDead_) {
		hitParticleLifeTime--;
		if (hitParticleLifeTime <= 0) {
			canErase_ = true;
		}
	}
}

void RapidBullet::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	render.DrawParticle(particle_.get(), BlendMode::Add);
	render.DrawParticle(hitParticle_.get(), BlendMode::Add);
}

void RapidBullet::Hit() {
	auto& ctx = GameContext::GetInstance();

	noHitTimer_->Start(0.15f);
	if (particleField_) {
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