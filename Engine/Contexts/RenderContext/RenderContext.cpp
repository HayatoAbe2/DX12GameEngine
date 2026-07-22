#include "RenderContext.h"
#include "Engine/Contexts/GameContext/GameContext.h"
#include "Engine/SceneObject/LightManager/LightManager.h"

RenderContext::RenderContext(Renderer* renderer, LightManager* lightManager) {
	renderer_ = renderer;
	lightManager_ = lightManager;
}

void RenderContext::DrawModel(Model* model, BlendMode blendMode) {
	renderer_->DrawModel(model, lightManager_, blendMode);
}

void RenderContext::DrawSprite(Sprite* sprite, BlendMode blendMode) {
	renderer_->DrawSprite(sprite, blendMode);
}

void RenderContext::DrawInstancedModel(InstancedModel* instancedModel, BlendMode blendMode) {
	renderer_->DrawModelInstance(instancedModel, lightManager_, blendMode);
}

void RenderContext::DrawParticle(ParticleSystem* particleSystem, BlendMode blendMode) {
	renderer_->DrawParticles(particleSystem, blendMode);
}
void RenderContext::DrawGPUParticle(ParticleSystem* particleSystem, BlendMode blendMode) {
	renderer_->DrawGPUParticle(particleSystem, blendMode);
}

void RenderContext::DrawPrimitive(Primitive* primitive, BlendMode blendMode) {
	renderer_->DrawPrimitive(primitive, blendMode);
}

void RenderContext::DrawSkybox(Texture* skybox) {
	renderer_->DrawSkybox(skybox);
}
