#pragma once
#include "Engine/Asset/Animation.h"
#include <memory>
#include <string>

class AnimationManager {
public:
	std::unique_ptr<Animation> Load(const std::string& directoryPath, const std::string& filename);
};

