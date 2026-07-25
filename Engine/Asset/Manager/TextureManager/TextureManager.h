#pragma once
#include "Engine/Asset/Resource/Texture.h"
#include <memory>
#include <unordered_map>
#include "externals/DirectXTex/DirectXTex.h"
#include <assimp/texture.h>

class DirectXContext;
class Logger;
class CommandListManager;
class DescriptorHeapManager;
class SRVManager;
class BufferManager;
class ConstantBufferManager;
class TextureManager {
public:
	TextureManager(DirectXContext* dxContext, Logger* logger);
	
	std::shared_ptr<Texture> Load(const std::string& texturePath, uint32_t id);
	void CreateTextureSRV(const std::shared_ptr<Texture>& texture, const aiTexture* tex = nullptr);

private:
	// 関数内で使う関数
	DirectX::ScratchImage LoadFile(const std::string& filePath, const aiTexture* tex);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	// デバイス
	ID3D12Device* device_ = nullptr;
	// ログ出力クラス
	Logger* logger_ = nullptr;
	// CommandList管理クラス
	CommandListManager* commandListManager_ = nullptr;
	// DescriptorHeap管理クラス
	DescriptorHeapManager* descriptorHeapManager_ = nullptr;
	// SRV管理クラス
	SRVManager* descriptorManager_ = nullptr;
	// バッファ管理クラス
	BufferManager* bufferManager_ = nullptr;
	// CB管理クラス
	ConstantBufferManager* cbManager_ = nullptr;

	// テクスチャのキャッシュ
	std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache_;

	// バッファリソース
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> uploadBuffers_;
};

