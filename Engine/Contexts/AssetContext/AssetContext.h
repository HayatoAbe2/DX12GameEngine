#pragma once
#include "Engine/SceneObject/Model/Model.h"
#include "Engine/SceneObject/Model/InstancedModel.h"
#include "Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h"
#include "Engine/SceneObject/Primitive/Primitive.h"
#include "Engine/SceneObject/Sprite/Sprite.h"
#include <Engine/SceneObject/Video/VideoPlayer/VideoPlayer.h>
class AssetManager;

/// <summary>
/// モデルなどのロードを行う窓口
/// </summary>
class AssetContext {
public:
	AssetContext(AssetManager* assetManager);

	// モデル読み込み
	std::unique_ptr<Model> LoadModel(const std::string& directoryPath, const std::string& filename);

	// モデル読み込み(インスタンシング描画)
	std::unique_ptr<InstancedModel> LoadInstancedModel(const std::string& directoryPath, const std::string& filename, const int num);

	// パーティクル
	std::unique_ptr<ParticleSystem> CreateParticleSystem(std::unique_ptr<Material> material, int instanceNum);
	std::unique_ptr<ParticleSystem> CreateGPUParticleSystem(std::unique_ptr<Material> material, int instanceNum);

	// Primitive
	std::unique_ptr<Primitive> CreatePrimitive(std::unique_ptr<Material> material, PrimitiveShape shape = PrimitiveShape::Plane);

	// スプライト読み込み
	std::unique_ptr<Sprite> LoadSprite(const std::string& texturePath);
	std::unique_ptr<Sprite> LoadSprite(std::shared_ptr<Texture> texture);

	// テクスチャ読み込み
	std::shared_ptr<Texture> LoadTexture(const std::string& filePath);

	// マテリアル読み込み
	std::unique_ptr<Material> CreateMaterial(std::shared_ptr<Texture> texture);

	// アニメーション読み込み
	std::vector<std::shared_ptr<Animation>> LoadAnimation(const std::string& directoryPath, const std::string& filePath);

	// 動画スプライト読み込み
	std::unique_ptr<VideoPlayer> LoadVideo(const std::string& filePath, bool isLoop);

private:
	AssetManager* assetManager_ = nullptr;
};

