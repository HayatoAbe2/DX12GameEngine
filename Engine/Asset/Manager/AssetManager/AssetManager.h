#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "Engine/Asset/Manager/ModelManager/ModelManager.h"
#include "Engine/Asset/Manager/TextureManager/TextureManager.h"
#include "Engine/Asset/Manager/SpriteManager/SpriteManager.h"
#include "Engine/Asset/Manager/AnimationManager/AnimationManager.h"

class DirectXContext;
class Logger;

// ゲームリソース管理クラス
class AssetManager {
public:
	// コンストラクタ
	AssetManager(DirectXContext* dxContext, Logger* logger);

	// 読み込み
	std::unique_ptr<Model> LoadModelFile(const std::string& directoryPath, const std::string& filename, bool enableLighting = true);
	std::unique_ptr<InstancedModel> LoadModelFile(const std::string& directoryPath, const std::string& filename, const int numInstance, bool enableLighting = true);
	std::unique_ptr<ParticleSystem> CreateParticle(int numInstance);
	std::unique_ptr<Sprite> LoadSprite(const std::string& filePath);
	std::shared_ptr<Texture> LoadTexture(const std::string& filePath);
	std::unique_ptr<Material> CreateMaterial(std::shared_ptr<Texture> texture);
	std::shared_ptr<Animation> LoadAnimation(const std::string& directoryPath, const std::string& filePath);

private:
	uint32_t GenerateID();

	// テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager_ = nullptr;

	// モデルマネージャー
	std::unique_ptr<ModelManager> modelManager_ = nullptr;

	// スプライトマネージャー
	std::unique_ptr<SpriteManager> spriteManager_ = nullptr;

	// アニメーション
	std::unique_ptr<AnimationManager> animationManager_ = nullptr;

	uint32_t assetId_ = 0;
};

