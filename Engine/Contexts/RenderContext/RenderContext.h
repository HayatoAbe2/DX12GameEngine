#pragma once
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Pipeline/BlendMode.h"

class RenderContext {
public:
	RenderContext(Renderer* renderer, LightManager* lightManager);

	// モデル描画
	void DrawModel(Model* model, BlendMode blendMode = BlendMode::Normal);

	// スプライト描画
	void DrawSprite(Sprite* sprite, BlendMode blendMode = BlendMode::Normal);

	// モデルのインスタンシング描画
	void DrawInstancedModel(InstancedModel* instancedModel, BlendMode blendMode = BlendMode::Normal);

	// パーティクルシステムで管理してるパーティクルの描画
	void DrawParticle(ParticleSystem* particleSystem, BlendMode blendMode = BlendMode::Normal);
	
	// Skybox描画
	void DrawSkybox(Texture* skybox);

	// ポストエフェクト設定
	void SetPostEffectType(PostEffectType type) { renderer_->SetPostEffectType(type); }

	// カメラセット
	void SetCamera(Camera* camera) { renderer_->SetCamera(camera); }
private:
	Renderer* renderer_ = nullptr;
	LightManager* lightManager_ = nullptr;
};

