#pragma once
#include "Engine/Asset/Sprite/Sprite.h"
#include <memory>

class DirectXContext;
class Logger;
class CommandListManager;
class DescriptorHeapManager;
class SRVManager;
class BufferManager;
class ConstantBufferManager;
class TextureManager;

class SpriteManager {
public:
	SpriteManager(DirectXContext* dxContext, Logger* logger, TextureManager* textureManager);

	std::unique_ptr<Sprite> Load(std::string texturePath);

private:
	// デバイス
	ID3D12Device* device_ = nullptr;
	// ログ出力クラス
	Logger* logger_ = nullptr;
	// CommandList管理クラス
	CommandListManager* commandListManager_ = nullptr;
	// DescriptorHeap管理クラス
	DescriptorHeapManager* descriptorHeapManager_ = nullptr;
	// SRV管理クラス
	SRVManager* srvManager_ = nullptr;
	// バッファ管理クラス
	BufferManager* bufferManager_ = nullptr;
	// CB管理クラス
	ConstantBufferManager* cbManager_ = nullptr;
	// テクスチャ管理クラス
	TextureManager* textureManager_ = nullptr;
};

