#pragma once
#include "Engine/SceneObject/Model/Mesh.h"
#include "Engine/Asset/Resource/Material/Material.h"
#include <vector>
#include <memory>

// 読み込んだモデルの共通データ
class ModelData {
public:
	// メッシュ
	std::vector<MeshData> meshes;

	// 初期状態のマテリアル
	std::vector<std::unique_ptr<Material>> defaultMaterials_;

	// jointと頂点&weightの対応
	std::map<std::string, JointWeightData> JointWeights;

	// SkinCluster
	SkinClusterData skinClusterData_;
};