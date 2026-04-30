#include "FireBullet.h"
#include "GameCommon.h"
#include "Map/MapCheck.h" 

void FireBullet::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& light = ctx.Light();

	particle_ = std::make_unique<ParticleSystem>();
	particle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Fire", "fireEffect.obj", particleNum_));
	particle_->SetLifeTime(10);
	particle_->SetColor({ 0.3f, 0.03f, 0.0f, 1.0f });

	explosionParticle_ = std::make_unique<ParticleSystem>();
	explosionParticle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Fire", "fireEffect.obj", particleNum_));
	explosionParticle_->SetLifeTime(10);
	explosionParticle_->SetColor({ 0.7f, 0.03f, 0.0f, 1.0f });

	particleField_ = std::make_unique<ParticleField>();
	particleField_->SetCheckArea(false);

	lightIndex_ = light.AddPointLight();
	auto& fireLight = light.GetPointLight(lightIndex_);
	fireLight.radius = 1.5f;
	fireLight.color = { 1,1,1,1 };
}

void FireBullet::Update(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

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
		for (int i = 0; i < 20; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform = model_->GetTransform();
			transform.translate += randomVector + velocity_ * 0.5f;
			transform.scale = model_->GetTransform().scale * 2.0f;
			particle_->Emit(transform, -velocity_ * 0.2f);
		}
	}
	particle_->Update();

	if (!isDead_) {
		auto& fireLight = light.GetPointLight(lightIndex_);
		fireLight.position = model_->GetTransform().translate;
	} else {
		explosionParticle_->Update();
		explosionEndLifeTime--;
		if (explosionEndLifeTime <= 0) {
			canErase_ = true;
		}
	}
}

void FireBullet::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();
	auto& render = ctx.Render();

	if (isDead_ && !shaked_ && lifeTime_ > 0) {
		camera->StartShake(2.0f, 3);
		shaked_ = true;
	}

	// パーティクル
	render.DrawParticle(particle_.get(), camera, BlendMode::Add);
	render.DrawParticle(explosionParticle_.get(), camera, BlendMode::Add);
}

void FireBullet::Hit() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();
	auto& audio = ctx.Audio();

	if (particleField_) {
		isDead_ = true;
		light.RemovePointLight(lightIndex_);
		if (lifeTime_ > 0) { // 自然消滅のときは音を鳴らさない
			audio.SoundPlay(L"Resources/Sounds/SE/explosion.mp3", false);
		}

		// 爆発開始
		particleField_->SetGravity(-0.6f, model_->GetTransform().translate);
		explosionParticle_->AddField(std::move(particleField_));
		for (int i = 0; i < explosionParticleNum_; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			};
			Transform transform = model_->GetTransform();
			transform.translate += randomVector;
			transform.scale = { 1.5f,1.5f,1.5f };
			explosionParticle_->Emit(transform, {});
		}
	}
}