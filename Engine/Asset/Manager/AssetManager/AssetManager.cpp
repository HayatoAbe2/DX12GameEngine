#include "AssetManager.h"

AssetManager::AssetManager(DirectXContext* dxContext, Logger* logger) {
	textureManager_ = std::make_unique<TextureManager>(dxContext, logger);
	modelManager_ = std::make_unique<ModelManager>(dxContext, logger, textureManager_.get());
	spriteManager_ = std::make_unique<SpriteManager>(dxContext, logger, textureManager_.get());
	animationManager_ = std::make_unique<AnimationManager>();
}

std::unique_ptr<Model> AssetManager::LoadModelFile(const std::string& directoryPath, const std::string& filename, bool enableLighting) {
	auto object = modelManager_->Load(GenerateID(), GenerateID(), GenerateID(), GenerateID(), directoryPath, filename, enableLighting);
	object->name = filename;
	return std::move(object);
}

std::unique_ptr<InstancedModel> AssetManager::LoadModelFile(const std::string& directoryPath, const std::string& filename, const int numInstance, bool enableLighting) {
	auto object = modelManager_->Load(GenerateID(), GenerateID(), GenerateID(), GenerateID(), directoryPath, filename, numInstance, enableLighting);
	object->name = filename;
	return std::move(object);
}

std::unique_ptr<ParticleSystem> AssetManager::CreateParticle(int numInstance) {
	return std::move(modelManager_->CreateParticleInstanceResource(numInstance, GenerateID()));
}

std::unique_ptr<Sprite> AssetManager::LoadSprite(const std::string& filePath) {
	auto object = spriteManager_->Load(filePath, GenerateID(), GenerateID(), GenerateID());
	object->name = filePath;
	return std::move(object);
}

std::shared_ptr<Texture> AssetManager::LoadTexture(const std::string& filePath) {
	auto resource = textureManager_->Load(filePath, GenerateID());
	resource->name_ = filePath;
	return resource;
}

std::unique_ptr<Material> AssetManager::CreateMaterial(std::shared_ptr<Texture> texture) {
	auto resource = modelManager_->LoadMaterial(texture, GenerateID(), GenerateID());
	resource->name_ = texture->name_;
	return std::move(resource);
}

std::shared_ptr<Animation> AssetManager::LoadAnimation(const std::string& directoryPath, const std::string& filePath) {
	auto resource = animationManager_->Load(directoryPath, filePath, GenerateID());
	resource->name_ = filePath;
	return resource;
}

uint32_t AssetManager::GenerateID() {
	return assetId_++;
}
