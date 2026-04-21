#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "Engine/Asset/Manager/ModelManager/ModelManager.h"
#include "Engine/Asset/Manager/TextureManager/TextureManager.h"
#include "Engine/Asset/Manager/SpriteManager/SpriteManager.h"

class DirectXContext;
class Logger;

// ゲームリソース管理クラス
class AssetManager {
public:
	// コンストラクタ
	AssetManager(DirectXContext* dxContext, Logger* logger);

	// 読み込み
	std::unique_ptr<Model> LoadModelFile(const std::string& directoryPath, const std::string& filename, bool enableLighting = true) {
		return std::move(modelManager_->Load(directoryPath, filename, enableLighting));
	}
	std::unique_ptr<InstancedModel> LoadModelFile(const std::string& directoryPath, const std::string& filename, const int numInstance, bool enableLighting = true) {
		return std::move(modelManager_->Load(directoryPath, filename, numInstance, enableLighting));
	}
	std::unique_ptr<Sprite> LoadSprite(const std::string& filePath) {
		return std::move(spriteManager_->Load(filePath));
	}


private:
	// テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager_ = nullptr;

	// モデルマネージャー
	std::unique_ptr<ModelManager> modelManager_ = nullptr;

	// スプライトマネージャー
	std::unique_ptr<SpriteManager> spriteManager_ = nullptr;
};

