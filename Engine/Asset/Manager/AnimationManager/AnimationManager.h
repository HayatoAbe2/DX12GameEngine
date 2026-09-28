#pragma once
#include "Engine/Asset/Resource/Animation.h"
#include <memory>
#include <string>
#include <functional>

class AnimationManager {
public:
	std::vector<std::shared_ptr<Animation>> Load(const std::string& directoryPath, const std::string& filename, const std::function<uint32_t()>& generateID);
};

