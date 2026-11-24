#pragma once
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Pipeline/BlendMode.h"

/// <summary>
/// 描画関数を呼び出す窓口
/// </summary>
class RenderContext {
public:
	RenderContext(Renderer* renderer, LightManager* lightManager);

	// モデル描画(shaderを渡すとそのシェーダーで描画。省略時は標準のシェーダー)
	void DrawModel(Model* model, BlendMode blendMode = BlendMode::Normal, ShaderHandle shader = {});

	// スプライト描画
	void DrawSprite(Sprite* sprite, BlendMode blendMode = BlendMode::Normal);

	// モデルのインスタンシング描画(shaderを渡すとそのシェーダーで描画。省略時は標準のシェーダー)
	void DrawInstancedModel(InstancedModel* instancedModel, BlendMode blendMode = BlendMode::Normal, ShaderHandle shader = {});

	// パーティクルシステムで管理しているパーティクルの描画
	void DrawParticle(ParticleSystem* particleSystem, BlendMode blendMode = BlendMode::Normal);
	void DrawGPUParticle(ParticleSystem* particleSystem, BlendMode blendMode = BlendMode::Normal);
	
	// 図形
	void DrawPrimitive(Primitive* primitive, BlendMode blendMode = BlendMode::Normal);

	// Skybox描画
	void DrawSkybox(Texture* skybox);

	// シェーダー登録(初期化時に1回行う。登録済みの名前なら登録済みのハンドルを返す)
	ShaderHandle RegisterShader(const std::string& name, const ShaderDesc& desc) { return renderer_->RegisterShader(name, desc); }
	// 登録名からシェーダーを取得(未登録なら無効なハンドル)
	ShaderHandle FindShader(const std::string& name) { return renderer_->FindShader(name); }

	// ポストエフェクト設定
	void AddPostEffect(PostEffectType type) { renderer_->AddPostEffect(type); }
	void SetDissolveMask(D3D12_GPU_DESCRIPTOR_HANDLE handle) { renderer_->SetDissolveMask(handle); }

	// カメラセット
	void SetCamera(Camera* camera) { renderer_->SetCamera(camera); }
	Camera* GetCamera() { return renderer_->GetCamera(); }

	// レンダーパス変更
	void BeginPass(RenderPass pass, bool clear = true) { renderer_->BeginPass(pass, clear); }
private:
	Renderer* renderer_ = nullptr;
	LightManager* lightManager_ = nullptr;
};

