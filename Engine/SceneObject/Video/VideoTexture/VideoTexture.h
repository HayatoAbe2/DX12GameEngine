#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <memory>
#include "Engine/Asset/Resource/Texture.h"

class DirectXContext;
class CommandListManager;
class SRVManager;
class BufferManager;

// 毎フレーム内容を書き換えられるテクスチャ(動画用)
class VideoTexture final {
public:
	explicit VideoTexture(DirectXContext* dxContext);
	~VideoTexture() { Release(); }
	VideoTexture(const VideoTexture&) = delete;
	VideoTexture& operator=(const VideoTexture&) = delete;

	bool Create(uint32_t width, uint32_t height, uint32_t textureId);
	void Release();

	// BGRA(1行あたり幅*4バイトで詰めたもの)を転送するコマンドを積む
	void Upload(const uint8_t* pixels);

	const std::shared_ptr<Texture>& GetTexture() const { return texture_; }
	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }

private:
	// 形式 X8 は、サンプリング時にαが常に1になる(RGB32 のα不定値を無視できる)
	static constexpr DXGI_FORMAT kFormat = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

	ID3D12Device* device_ = nullptr;
	CommandListManager* commandListManager_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	BufferManager* bufferManager_ = nullptr;

	std::shared_ptr<Texture> texture_;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;
	uint8_t* mapped_ = nullptr;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_{};
	uint32_t srvIndex_ = UINT32_MAX;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	bool everUploaded_ = false;
};