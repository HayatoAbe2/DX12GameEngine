#pragma once
#include <vector>
#include <memory>
#include "Engine/SceneObject/SceneObject.h"

class Scene {
public:
	std::vector<std::unique_ptr<SceneObject>> sceneObjects_;
};