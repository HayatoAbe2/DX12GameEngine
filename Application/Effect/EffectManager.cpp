 #include "EffectManager.h"
#include <numbers>

void EffectManager::Initialize() {
}

void EffectManager::Update() {
	for (auto& effect : hitEffect_) {
		// 拡大
		effect->SetScale(effect->GetTransform().scale + Vector3{ hitEffectUpScaleSpeed_, hitEffectUpScaleSpeed_, hitEffectUpScaleSpeed_});
		
		// 薄くする
		MaterialData data = effect->GetData()->defaultMaterials_[0]->GetData();
		data.color.w -= 0.1f;
		effect->GetData()->defaultMaterials_[0]->SetData(data);
	}

	// 消滅したエフェクトの削除
	hitEffect_.erase(
		std::remove_if(hitEffect_.begin(), hitEffect_.end(),
			[](const std::unique_ptr<Model>& effect) {
				return effect->GetData()->defaultMaterials_[0]->GetData().color.w <= 0.0f;
			}
		),
		hitEffect_.end()
	);
}

void EffectManager::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	for (const auto& effect : hitEffect_) {
		Vector3 rotate = camera->transform_.rotate;
		rotate.x += float(std::numbers::pi);
		effect->SetRotate(rotate);
		render.DrawModel(effect.get(), camera, BlendMode::Add);
	}
}

void EffectManager::SpawnHitEffect(const Vector3& pos) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	auto effect = std::make_unique<Model>();
	effect = asset.LoadModel("Resources/HitEffect", "hitEffect.obj");
	effect->SetTransform({ {0.5f,0.5f,0.5f},{},pos });
	hitEffect_.push_back(std::move(effect));
}

void EffectManager::SpawnSoulEffect(const Vector3& spawnPos, const Vector3& goalPos) {

}
