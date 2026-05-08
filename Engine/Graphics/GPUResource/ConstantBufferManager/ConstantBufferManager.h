#pragma once
#include "Engine/Graphics/GPUData/TransformationMatrix.h"
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <vector>

class ConstantBufferManager {
public:
	ConstantBufferManager(ID3D12Device* device);

	// GPU書き込み&CBVアドレスを返す
	D3D12_GPU_VIRTUAL_ADDRESS UploadTransform(const TransformationMatrix& data);

	void BeginFrame();
private:
	// トランスフォームバッファ初期化
	void InitializeTransformCB(ID3D12Device* device);

	// transform
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;
	UINT8* mappedTransformData_ = nullptr;

	// CBサイズ
	static constexpr UINT kCBSize = (sizeof(TransformationMatrix) + 255) & ~255;
	const UINT kMaxObjects = 4096; // 最大数

	uint32_t currentCBOffset_ = 0;
};

