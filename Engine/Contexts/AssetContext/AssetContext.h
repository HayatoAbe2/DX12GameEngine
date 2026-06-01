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

	// パーティクル
	std::unique_ptr<ParticleSystem> CreateParticleSystem(ParticleShape shape, std::unique_ptr<Material> material, int instanceNum);

	// スプライト読み込み
	std::unique_ptr<Sprite> LoadSprite(const std::string& texturePath);

	// テクスチャ読み込み
	std::shared_ptr<Texture> LoadTexture(const std::string& filePath);

	// マテリアル読み込み
	std::unique_ptr<Material> CreateMaterial(std::shared_ptr<Texture> texture);

	// アニメーション読み込み
	std::shared_ptr<Animation> LoadAnimation(const std::string& directoryPath, const std::string& filePath);

	//void SoundLoad(const wchar_t* filename);

private:
	AssetManager* assetManager_ = nullptr;
};

