#include "ChargeEffect.h"

ChargeEffect::ChargeEffect(Vector4 color) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// パーティクル
	particle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), kParticleCount_);
	particle_->SetLifeTime(5);
	particle_->SetColor(color);
}

void ChargeEffect::Update(Vector2 center, float radius) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	for (int i = 0; i < kParticleCount_ / 10; ++i) {
		Vector3 randomVector = {
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
		};
		Transform transform = {
			Vector3{radius, radius, radius} * 2.0f,
			{0,0,0},
			Vector3{center.x, 0.5f, center.y} + randomVector };
		particle_->Emit(transform, Vector3{});
	}
	particle_->Update();
}

void ChargeEffect::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// パーティクル
	render.DrawParticle(particle_.get(), BlendMode::Add);
}
