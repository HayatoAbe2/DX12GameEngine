#pragma once
#include <vector>
#include <memory>
#include "Engine/Asset/Resource/Resource.h"
#include "Engine/Scene/BaseScene/BaseScene.h"

class SceneEditor {
public:
	void Update();
	void Draw();

	std::vector<std::unique_ptr<Resource>> resources_;
	BaseScene* scene_;
	SceneObject* selected_;
};

