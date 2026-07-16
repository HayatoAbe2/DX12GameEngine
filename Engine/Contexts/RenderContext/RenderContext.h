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

	// パーティクルシステムで管理しているパーティクルの描画
	void DrawParticle(ParticleSystem* particleSystem, BlendMode blendMode = BlendMode::Normal);
	void DrawGPUParticle(ParticleSystem* particleSystem, BlendMode blendMode = BlendMode::Normal);
	
	// 図形
	void DrawPrimitive(Primitive* primitive, BlendMode blendMode = BlendMode::Normal);

	// Skybox描画
	void DrawSkybox(Texture* skybox);

	// ポストエフェクト設定
	void SetPostEffectType(PostEffectType type) { renderer_->SetPostEffectType(type); }
	void SetDissolveMask(D3D12_GPU_DESCRIPTOR_HANDLE handle) { renderer_->SetDissolveMask(handle); }

	// カメラセット
	void SetCamera(Camera* camera) { renderer_->SetCamera(camera); }
	Camera* GetCamera() { return renderer_->GetCamera(); }
private:
	Renderer* renderer_ = nullptr;
	LightManager* lightManager_ = nullptr;
};

