#pragma once
#include <vector>
#include "SubMesh.h"

struct MeshRuntime {
	// サブメッシュ
	std::vector<SubMeshRuntime> subMeshes{};
};


struct MeshData {
	// サブメッシュ
	std::vector<SubMeshData> subMeshes{};
};
