#pragma once
#include "Engine/Graphics/GPUData/MaterialData.h"
#include "Engine/Asset/Texture.h"
#include <wrl.h>
#include <memory>

class BufferManager;
class Material {
public:

    void Initialize(BufferManager* bufferManager, bool useTexture, bool enableLighting);
    void UpdateGPU(); // GPUに転送

    void SetData(MaterialData data) { data_ = data; }
	const MaterialData GetData()const { return data_; }

    Microsoft::WRL::ComPtr<ID3D12Resource> GetCBV() { return constantBuffer_; }

	void SetTexture(std::shared_ptr<Texture> texture) { 
		texture_ = texture;
	}
	std::shared_ptr<Texture> GetTexture() { return texture_; }

	void SetEnvironmentTexture(std::shared_ptr<Texture> texture) {
		environmentTexture_ = texture;
	}
	std::shared_ptr<Texture> GetEnvironmentTexture() { return environmentTexture_; }

	void SwitchLighting(bool enableLighting) {
		data_.enableLighting = enableLighting;
	}

	const D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSRVHandle() const {
		if (texture_) {
			return texture_->GetSRVHandle();
		}
		return { 0 };
	}
	const D3D12_GPU_DESCRIPTOR_HANDLE GetEnvironmentTextureSRVHandle() const {
		if (environmentTexture_) {
			return environmentTexture_->GetSRVHandle();
		}
		return { 0 };
	}

private:
    MaterialData data_;  // CPU側のデータ
    MaterialData* mappedPtr_ = nullptr; // GPUメモリに直接アクセスするためのポインタ
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_ = nullptr; // GPU側リソース
	std::shared_ptr<Texture> texture_ = nullptr; // テクスチャ
	std::shared_ptr<Texture> environmentTexture_ = nullptr; // テクスチャ
	D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle_;
};
