#pragma once
#include <vector>
#include <memory>
#include "Engine/Asset/Resource/Resource.h"

class SceneEditor {
public:
	std::vector<std::unique_ptr<Resource>> resources_;
};

