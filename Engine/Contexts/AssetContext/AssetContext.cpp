#include "AssetContext.h"
#include "Engine/Asset/Manager/AssetManager/AssetManager.h"

AssetContext::AssetContext(AssetManager* assetManager) {
	assetManager_ = assetManager;
}

std::unique_ptr<Model> AssetContext::LoadModel(const std::string& directoryPath, const std::string& filename) {
	return std::move(assetManager_->LoadModelFile(directoryPath, filename));
}

std::unique_ptr<InstancedModel> AssetContext::LoadInstancedModel(const std::string& directoryPath, const std::string& filename, const int num) {
	return std::move(assetManager_->LoadModelFile(directoryPath, filename, num));
}

std::unique_ptr<ParticleSystem> AssetContext::CreateParticleSystem(std::unique_ptr<Material> material, int numInstance) {
	std::unique_ptr<ParticleSystem> particleSystem = assetManager_->CreateParticle(numInstance);
	particleSystem->name = material->name_;
	particleSystem->Initialize(std::move(material), numInstance);
	return std::move(particleSystem);
}
std::unique_ptr<ParticleSystem> AssetContext::CreateGPUParticleSystem(std::unique_ptr<Material> material, int numInstance) {
	std::unique_ptr<ParticleSystem> particleSystem = assetManager_->CreateGPUParticle(numInstance);
	particleSystem->name = material->name_;
	particleSystem->Initialize(std::move(material), numInstance);
	return std::move(particleSystem);
}

std::unique_ptr<Primitive> AssetContext::CreatePrimitive(std::unique_ptr<Material> material, PrimitiveShape shape) {
	std::unique_ptr<Primitive> primitive = assetManager_->CreatePrimitive(shape);
	auto data = material->GetData();
	data.enableLighting = false;
	material->SetData(data);

	primitive->Initialize(std::move(material));
	primitive->shape_ = shape;
	return std::move(primitive);
}

std::unique_ptr<Sprite> AssetContext::LoadSprite(const std::string& texturePath) {
	return std::move(assetManager_->LoadSprite(texturePath));
}

std::shared_ptr<Texture> AssetContext::LoadTexture(const std::string& filePath) {
	return assetManager_->LoadTexture(filePath);
}

std::unique_ptr<Material> AssetContext::CreateMaterial(std::shared_ptr<Texture> texture) {
	return assetManager_->CreateMaterial(texture);
}

std::shared_ptr<Animation> AssetContext::LoadAnimation(const std::string& directoryPath, const std::string& filePath) {
	return assetManager_->LoadAnimation(directoryPath, filePath);
}