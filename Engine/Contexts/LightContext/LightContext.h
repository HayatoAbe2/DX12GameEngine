#pragma once
#include "Engine/Object/LightManager.h"
class LightContext {
public:
	LightContext(LightManager* lightManager);

	int AddPointLight();
	void RemovePointLight(int index);
	PointLight& GetPointLight(int index);
	int AddSpotLight();
	void RemoveSpotLight(int index);
	SpotLight& GetSpotLight(int index);
	void DrawLightImGui();

private:
	LightManager* lightManager_ = nullptr;
};

