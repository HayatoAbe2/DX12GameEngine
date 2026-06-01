#pragma once
#include "Engine/Asset/Animation.h"
#include <memory>
#include <string>

class AnimationManager {
public:
	std::shared_ptr<Animation> Load(const std::string& directoryPath, const std::string& filename);
};

