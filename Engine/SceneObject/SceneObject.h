#pragma once
#include <string>

/// <summary>
/// シーン上の描画オブジェクト
/// </summary>
class SceneObject {
public:
	explicit SceneObject(uint32_t id) : id_(id) {}
	virtual ~SceneObject() = default;

	const uint32_t GetID() const { return id_; }

	// 名前(エディタで表示)
	std::string name;

	// タグ(ゲーム側で自由に利用可能)
	std::string tag;

protected:
	const uint32_t id_;
};