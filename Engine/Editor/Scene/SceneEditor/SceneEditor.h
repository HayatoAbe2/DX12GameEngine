#pragma once
#include <vector>
#include <memory>
#include "Engine/Asset/Resource/Resource.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#include <Externals/nlohmann/json.hpp>
#include "GizmoCtx.h"

class SceneEditor {
public:
	void Update();
	void Draw(Camera* camera);

	void DrawInspector(SceneObject* object);

	void Save();
	void Load(const std::string& path);

	std::vector<std::unique_ptr<Resource>> resources_;
	BaseScene* scene_;
	SceneObject* selected_;
	std::string selectedAssetDir_[2] = { "Resources/Debug/human", "Resources/Floor"};
	std::string selectedAssetPath_[2] = { "walk.gltf", "floor.obj" };
	int selectedAssetIndex_ = 0;
	int gizmoMode_ = 0;
	GizmoCtx gizmoCtx_{};
};

