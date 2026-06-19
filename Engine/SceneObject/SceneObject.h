#pragma once
#include <string>

class SceneObject {
public:
	explicit SceneObject(uint32_t id) : id_(id) {}

	uint32_t GetID() { return id_; }

protected:
	uint32_t id_;
	std::string name_;
};