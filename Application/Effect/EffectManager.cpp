 #include "EffectManager.h"
#include <numbers>

void EffectManager::Initialize() {
}

void EffectManager::Update() {
	for (auto& effect : hitEffect_) {
		// 拡大
		effect->SetScale(effect->GetTransform().scale + Vector3{ hitEffectUpScaleSpeed_, hitEffectUpScaleSpeed_, hitEffectUpScaleSpeed_});
		
		// 薄くする
		MaterialData data = effect->GetMaterial(0)->GetData();
		data.color.w -= 0.1f;
		effect->GetMaterial(0)->SetData(data);
	}
	for (const auto& spark : hitSpark_) {
		spark->Update();
	}
	for (const auto& explode : explode_) {
		explode->Update();
	}

	// 消滅したエフェクトの削除
	hitEffect_.erase(
		std::remove_if(hitEffect_.begin(), hitEffect_.end(),
			[](const std::unique_ptr<Model>& effect) {
				return effect->GetMaterial(0)->GetData().color.w <= 0.0f;
			}
		),
		hitEffect_.end()
	);
	hitSpark_.erase(
		std::remove_if(hitSpark_.begin(), hitSpark_.end(),
			[](const std::unique_ptr<HitEffect>& effect) {
				return effect->IsFinished();
			}
		),
		hitSpark_.end()
	);
	explode_.erase(
		std::remove_if(explode_.begin(), explode_.end(),
			[](const std::unique_ptr<ExplodeEffect>& effect) {
				return effect->IsFinished();
			}
		),
		explode_.end()
	);
}

void EffectManager::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	for (const auto& effect : hitEffect_) {
		Vector3 rotate = camera->transform_.rotate;
		rotate.x += float(std::numbers::pi);
		effect->SetRotate(rotate);
		render.DrawModel(effect.get(), BlendMode::Add);
	}

	for (const auto& spark : hitSpark_) {
		spark->Draw(camera);
	}
	for (const auto& explode : explode_) {
		explode->Draw(camera);
	}
}

void EffectManager::SpawnHitEffect(const Vector3& pos) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	auto effect = asset.LoadModel("Resources/HitEffect", "hitEffect.obj");
	effect->SetTransform({ {0.5f,0.5f,0.5f},{},pos });
	hitEffect_.push_back(std::move(effect));

	auto spark = std::make_unique<HitEffect>();
	spark->Initialize(pos, { 1,1,1,1 });
	hitSpark_.push_back(std::move(spark));
}

void EffectManager::SpawnExplodeEffect(const Vector3& pos) {
	auto explode = std::make_unique<ExplodeEffect>();
	explode->Initialize(pos, { 1,1,1,1 }, 3.0f);
	explode_.push_back(std::move(explode));
}