#pragma once
#include <wrl.h>
#include <d3d12.h>
class BufferManager {
public:
	BufferManager(ID3D12Device* device);
	// バッファリソース作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateUploadBuffer(size_t sizeInBytes);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);
private:
	ID3D12Device* device_ = nullptr;
};

