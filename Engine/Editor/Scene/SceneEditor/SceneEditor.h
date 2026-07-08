#pragma once
#include <vector>
#include <memory>
#include "Engine/Asset/Resource/Resource.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#ifdef USE_IMGUI
#include <Externals/nlohmann/json.hpp>
#include "GizmoCtx.h"
#endif

class SceneEditor {
public:
	void Update();
	void Draw(Camera* camera);

	void DrawInspector(SceneObject* object);

	void Save();
	void Load(const std::string& path);

	std::string SerializeScene();
	void DeserializeScene(const std::string& jsonText);

	void PushUndo();
	void Undo();
	void Redo();

	// 選択
	void ClickSelect();

	std::vector<std::unique_ptr<Resource>> resources_;
	BaseScene* scene_;
	SceneObject* selected_;
	std::string selectedAssetDir_[2] = { "Resources/Debug/human", "Resources/Floor"};
	std::string selectedAssetPath_[2] = { "walk.gltf", "floor.obj" };
	int selectedAssetIndex_ = 0;
	int gizmoMode_ = 0;
#ifdef USE_IMGUI
	GizmoCtx gizmoCtx_{};
#endif

	std::vector<std::string> undoStack_;
	std::vector<std::string> redoStack_;
	bool wasUsing_ = false;

	bool useSnap_ = false;
	float moveSnap_ = 1.0f;
	float rotateSnap_ = 15.0f;
	float scaleSnap_ = 0.1f;

	int editingInstance_ = 0;

	bool undoRequest_ = false;
	bool redoRequest_ = false;
};

