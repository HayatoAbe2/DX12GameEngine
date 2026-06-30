#pragma once
#include <string>

class SceneObject {
public:
	explicit SceneObject(uint32_t id) : id_(id) {}
	virtual ~SceneObject() = default;

	uint32_t GetID() { return id_; }

	std::string name;
	std::string tag;
protected:
	uint32_t id_;
};