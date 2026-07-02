#pragma once
#include "Engine/SceneObject/SceneObject.h"
#include "Engine/SceneObject/Transform.h"
#include "Engine/Asset/Resource/Material/Material.h"

enum class PrimitiveShape {
	Plane,
	Ring,
	Cylinder,
};

class Primitive : public SceneObject {
public:
	Primitive(uint32_t id) : SceneObject(id) {};
	void Initialize(std::unique_ptr<Material> material) {
		material_ = std::move(material);
	}
	Material* GetMaterial() { return material_.get(); }

	// トランスフォーム
	Transform transform_;

	// 形
	PrimitiveShape shape_ = PrimitiveShape::Plane;
private:
	std::unique_ptr<Material> material_;
};