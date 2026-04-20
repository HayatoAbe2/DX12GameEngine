#pragma once
#include "Engine/Asset/Model/Mesh.h"
#include "Engine/Asset/Material/Material.h"
#include <vector>
#include <memory>

// 読み込んだモデルの共通データ
class ModelData {
public:
	// メッシュ
	std::vector<std::unique_ptr<Mesh>> meshes;

	// 初期状態のマテリアル
	std::vector<std::unique_ptr<Material>> defaultMaterials_;
};