#include "AssetContext.h"

AssetContext::AssetContext(AssetManager* assetManager) {
	assetManager_ = assetManager;
}

std::unique_ptr<Model> AssetContext::LoadModel(const std::string& directoryPath, const std::string& filename, bool enableLighting) {
	return std::move(assetManager_->LoadModelFile(directoryPath, filename, enableLighting));
}

std::unique_ptr<InstancedModel> AssetContext::LoadInstancedModel(const std::string& directoryPath, const std::string& filename, const int num) {
	return std::move(assetManager_->LoadModelFile(directoryPath, filename, num));
}

//std::unique_ptr<Sprite> AssetContext::LoadSprite(std::string texturePath) {
//	return std::move(assetManager_->LoadSprite(texturePath));
//	return nullptr;
//}