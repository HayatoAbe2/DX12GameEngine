#pragma once
#include <string>

class SceneObject {
public:
	explicit SceneObject(uint32_t id) : id_(id) {}
	virtual ~SceneObject() = default;

	uint32_t GetID() { return id_; }

	// 名前(エディタで表示)
	std::string name;

	// タグ(ゲーム側で自由に利用可能)
	std::string tag;

	int groupID;
protected:
	uint32_t id_;
};