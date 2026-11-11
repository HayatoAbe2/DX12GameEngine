#pragma once
#include <vector>
#include <memory>
#include "Engine/Asset/Resource/Resource.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#include <Externals/nlohmann/json.hpp>

#ifdef USE_IMGUI
#include "imgui_stdlib.h"
#include "../SceneEditor/GizmoCtx.h"
#endif

struct SpawnRate {
    std::string name;
    float weight = 0.0f;
};

class SpawnRateEditor {
public:
	SpawnRateEditor();
	void Draw(const std::vector<std::string>& enemyNames);

private:
    void Save(const std::string& filePath);
    void Load(const std::string& filePath);

    // 編集
    void AddSpawnRate();
    void RemoveSpawnRate(size_t index);

    std::vector<SpawnRate> spawnRates_;

    // 編集対象ファイル
    std::string filePath_ = "Resources/Data/Spawn/Enemy/Standard.json";
};

