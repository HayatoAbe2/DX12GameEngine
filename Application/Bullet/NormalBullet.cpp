#include "NormalBullet.h"
#include "Map/MapCheck.h"

void NormalBullet::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	particle_ = std::make_unique<ParticleSystem>();
	particle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Fire", "fireEffect.obj", particleNum_));
	particle_->SetLifeTime(2);
	particle_->SetColor({ 0.5f, 0.7f, 0.0f, 1.0f });

	hitParticle_ = std::make_unique<ParticleSystem>();
	hitParticle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Fire", "fireEffect.obj", hitParticleNum_));
	hitParticle_->SetLifeTime(hitParticleLifeTime);
	hitParticle_->SetColor({ 0.5f, 0.7f, 0.0f, 1.0f });
	particleField_ = std::make_unique<ParticleField>();
	particleField_->SetCheckArea(false);
}

void NormalBullet::Update(MapCheck* mapCheck) {
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

		if (mapCheck->IsHitWall(pos, status_.bulletSize / 2.0f)) {
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

void NormalBullet::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	render.DrawParticle(particle_.get(), camera, BlendMode::Add);
	render.DrawParticle(hitParticle_.get(), camera, BlendMode::Add);
}

void NormalBullet::Hit() {
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