#pragma once
#include "Engine/Scene/BaseScene/BaseScene.h"
#include <string>
#include <memory>

class SceneFactoryBase {
public:
	virtual ~SceneFactoryBase() = default;
	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};

