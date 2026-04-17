#include "LightContext.h"

LightContext::LightContext(LightManager* lightManager) {
	lightManager_ = lightManager;
}

int LightContext::AddPointLight() {
	return lightManager_->AddPointLight();
}

void LightContext::RemovePointLight(int index) {
	lightManager_->RemovePointLight(index);
}

PointLight& LightContext::GetPointLight(int index) {
	return lightManager_->GetPointLight(index);
}

int LightContext::AddSpotLight() {
	return lightManager_->AddSpotLight();
}

void LightContext::RemoveSpotLight(int index) {
	lightManager_->RemoveSpotLight(index);
}

SpotLight& LightContext::GetSpotLight(int index) {
	return lightManager_->GetSpotLight(index);
}

void LightContext::DrawLightImGui() {
	lightManager_->DrawImGui();
}