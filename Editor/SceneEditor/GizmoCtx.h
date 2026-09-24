#pragma once
#ifdef USE_IMGUI
#include <Externals/ImGuizmo/ImGuizmo.h>
#include "Engine/SceneObject/SceneObject.h"

struct GizmoCtx {
	bool isActive = false;
	float view[16]{};
	float proj[16]{};
	float modelMatrix[16]{};
	bool useSnap = false;
	float snap[3] = {1,1,1};
	ImGuizmo::OPERATION op;
	SceneObject* target;
	int editingInstance;
	bool editAllInstances;

	bool useRangeSelect = false;
	int minRange = 0;
	int maxRange = 0;
};
#endif