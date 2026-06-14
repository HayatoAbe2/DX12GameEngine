#include "AssetManager.h"

AssetManager::AssetManager(DirectXContext* dxContext, Logger* logger) {
	textureManager_ = std::make_unique<TextureManager>(dxContext, logger);
	modelManager_ = std::make_unique<ModelManager>(dxContext, logger, textureManager_.get());
	spriteManager_ = std::make_unique<SpriteManager>(dxContext, logger, textureManager_.get());
	animationManager_ = std::make_unique<AnimationManager>();
}

std::unique_ptr<Model> AssetManager::LoadModelFile(const std::string& directoryPath, const std::string& filename, bool enableLighting) {
	return std::move(modelManager_->Load(GenerateID(), GenerateID(), GenerateID(), GenerateID(), directoryPath, filename, enableLighting));
}

std::unique_ptr<InstancedModel> AssetManager::LoadModelFile(const std::string& directoryPath, const std::string& filename, const int numInstance, bool enableLighting) {
	return std::move(modelManager_->Load(GenerateID(), GenerateID(), GenerateID(), GenerateID(), directoryPath, filename, numInstance, enableLighting));
}

std::unique_ptr<ParticleSystem> AssetManager::CreateParticle(int numInstance) {
	return std::move(modelManager_->CreateParticleInstanceResource(numInstance, GenerateID()));
}

std::unique_ptr<Sprite> AssetManager::LoadSprite(const std::string& filePath) {
	return std::move(spriteManager_->Load(filePath, GenerateID(), GenerateID(), GenerateID()));
}

std::shared_ptr<Texture> AssetManager::LoadTexture(const std::string& filePath) {
	return textureManager_->Load(filePath, GenerateID());
}

std::unique_ptr<Material> AssetManager::CreateMaterial(std::shared_ptr<Texture> texture) {
	return std::move(modelManager_->LoadMaterial(texture, GenerateID(), GenerateID()));
}

std::shared_ptr<Animation> AssetManager::LoadAnimation(const std::string& directoryPath, const std::string& filePath) {
	return animationManager_->Load(directoryPath, filePath, GenerateID());
}

uint32_t AssetManager::GenerateID() {
	return assetId_++;
}
