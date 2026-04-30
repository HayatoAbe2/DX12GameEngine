#pragma once
#include "Engine/Asset/Manager/AssetManager/AssetManager.h"

// モデル
class AssetContext {
public:
	AssetContext(AssetManager* assetManager);

	// モデル読み込み
	std::unique_ptr<Model> LoadModel(const std::string& directoryPath, const std::string& filename, bool enableLighting = true);

	// モデル読み込み(インスタンシング描画)
	std::unique_ptr<InstancedModel> LoadInstancedModel(const std::string& directoryPath, const std::string& filename, const int num);

	// スプライト読み込み
	std::unique_ptr<Sprite> LoadSprite(std::string texturePath);

	//void SoundLoad(const wchar_t* filename);

private:
	AssetManager* assetManager_ = nullptr;
};

