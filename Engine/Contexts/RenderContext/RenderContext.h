#pragma once
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Pipeline/BlendMode.h"
class RenderContext {
public:
	RenderContext(Renderer* renderer, LightManager* lightManager);

	// モデル描画
	void DrawModel(Model* model, Camera* camera, BlendMode blendMode = BlendMode::Normal);

	// スプライト描画
	void DrawSprite(Sprite* sprite, BlendMode blendMode = BlendMode::Normal);

	// モデルのインスタンシング描画
	void DrawInstancedModel(InstancedModel* instancedModel, Camera* camera, BlendMode blendMode = BlendMode::Normal);

	// パーティクルシステムで管理してるパーティクルの描画
	void DrawParticle(ParticleSystem* particleSystem, Camera* camera, BlendMode blendMode = BlendMode::Normal);

private:
	Renderer* renderer_ = nullptr;
	LightManager* lightManager_ = nullptr;
};

