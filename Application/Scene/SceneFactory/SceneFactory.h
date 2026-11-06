#pragma once
#include "Engine/Scene/SceneFactoryBase/SceneFactoryBase.h"

/// <summary>
/// シーン作成
/// </summary>
class SceneFactory : public SceneFactoryBase{
public:
	std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;
};

