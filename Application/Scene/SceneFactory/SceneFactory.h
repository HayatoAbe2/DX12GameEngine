#pragma once
#include "Engine/Scene/SceneFactoryBase/SceneFactoryBase.h"
class SceneFactory : public SceneFactoryBase{
public:
	std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;
};

