#pragma once
#ifdef USE_IMGUI
#include <Externals/ImGuizmo/ImGuizmo.h>
#include "Engine/SceneObject/SceneObject.h"

struct GizmoCtx {
	bool isActive = false;
	float view[16]{};
	float proj[16]{};
	float model[16]{};
	ImGuizmo::OPERATION op;
	SceneObject* target;
};
#endif