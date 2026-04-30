#include "SoulEffect.h"

void SoulEffect::Initialize(Vector3 pos, Vector3 goal) {
	pos_ = pos;
	goal_ = goal;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	particle_ = std::make_unique<ParticleSystem>();
	particle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Fire", "fireEffect.obj", particleNum_));
	particle_->SetLifeTime(10);
	particle_->SetColor({ 0.8f, 0.0f, 0.8f, 1.0f });
}

void SoulEffect::Update() {
	auto& ctx = GameContext::GetInstance();

	// パーティクル
	for (int i = 0; i < 20; ++i) {
		Vector3 randomVector = {
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		};
		Transform transform;
		transform.translate += randomVector;
		particle_->Emit(transform, {});
	}
}

void SoulEffect::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawParticle(particle_.get(), camera, BlendMode::Add);
}