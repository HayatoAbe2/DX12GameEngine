#include "RenderContext.h"
#include "Engine/Contexts/GameContext/GameContext.h"
#include "Engine/Object/LightManager.h"

RenderContext::RenderContext(Renderer* renderer, LightManager* lightManager) {
	renderer_ = renderer;
	lightManager_ = lightManager;
}

void RenderContext::DrawModel(Model* model, Camera* camera, BlendMode blendMode) {
	renderer_->DrawModel(model, camera, lightManager_, static_cast<int>(blendMode));
}

void RenderContext::DrawSprite(Sprite* sprite, BlendMode blendMode) {
	renderer_->DrawSprite(sprite, static_cast<int>(blendMode));
}

void RenderContext::DrawInstancedModel(InstancedModel* instancedModel, Camera* camera, BlendMode blendMode) {
	renderer_->DrawModelInstance(instancedModel, camera, lightManager_, static_cast<int>(blendMode));
}

void RenderContext::DrawParticle(ParticleSystem* particleSystem, Camera* camera, BlendMode blendMode) {
	renderer_->DrawParticles(particleSystem, camera, static_cast<int>(blendMode));
}
