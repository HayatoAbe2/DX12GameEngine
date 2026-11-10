#include "VideoTexture.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"
#include "Engine/Graphics/Core/CommandListManager/CommandListManager.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"
#include <Externals/DirectXTex/d3dx12.h>
#include <cstring>

VideoTexture::VideoTexture(DirectXContext* dxContext) {
	device_ = dxContext->GetDeviceManager()->GetDevice().Get();
	commandListManager_ = dxContext->GetCommandListManager();
	srvManager_ = dxContext->GetSRVManager();
	bufferManager_ = dxContext->GetBufferManager();
}

bool VideoTexture::Create(uint32_t width, uint32_t height, uint32_t textureId) {
	Release();

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = kFormat;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	HRESULT hr = device_->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&resource_));
	if (FAILED(hr)) { return false; }

	// 転送元バッファ(1行ごとのアライメントは GetCopyableFootprints が教えてくれる)
	UINT numRows = 0;
	UINT64 rowSize = 0, totalBytes = 0;
	device_->GetCopyableFootprints(&desc, 0, 1, 0, &footprint_, &numRows, &rowSize, &totalBytes);
	uploadBuffer_ = bufferManager_->CreateUploadBuffer(static_cast<size_t>(totalBytes));
	uploadBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_)); // 常時マップしておく

	// SRV は1回作るだけ。以後は中身だけ書き換える
	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateTextureSRV(srvIndex_, resource_.Get(), kFormat, 1, false);

	texture_ = std::make_shared<Texture>(textureId);
	texture_->SetResource(resource_);
	texture_->SetSRVHandle(srvManager_->GetGPUHandle(srvIndex_));

	width_ = width;
	height_ = height;
	return true;
}

void VideoTexture::Release() {
	// 転送コマンドが未実行のまま解放しないよう、一度だけ完了を待つ
	// (動画の破棄は Update 中に行うこと。Draw 中に呼ぶと描画途中のコマンドごと実行される)
	if (everUploaded_) {
		commandListManager_->ExecuteAndWait();
		everUploaded_ = false;
	}
	if (uploadBuffer_ && mapped_) { uploadBuffer_->Unmap(0, nullptr); }
	mapped_ = nullptr;
	uploadBuffer_.Reset();
	resource_.Reset();
	texture_.reset();
	if (srvIndex_ != UINT32_MAX) {
		srvManager_->Free(srvIndex_);
		srvIndex_ = UINT32_MAX;
	}
	width_ = height_ = 0;
}

void VideoTexture::Upload(const uint8_t* pixels) {
	if (!resource_) { return; }

	// 1) CPU 側: 転送元バッファに行ごとに書き込む(行のピッチは GPU の要求に合わせる)
	const size_t srcPitch = static_cast<size_t>(width_) * 4;
	const size_t dstPitch = footprint_.Footprint.RowPitch;
	for (uint32_t y = 0; y < height_; ++y) {
		std::memcpy(mapped_ + footprint_.Offset + y * dstPitch, pixels + y * srcPitch, srcPitch);
	}

	// 2) GPU 側: 描画用コマンドリストにコピーを積む(待たない)
	auto cmdList = commandListManager_->GetCommandList();
	auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(resource_.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &toCopy);

	CD3DX12_TEXTURE_COPY_LOCATION dst(resource_.Get(), 0);
	CD3DX12_TEXTURE_COPY_LOCATION src(uploadBuffer_.Get(), footprint_);
	cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	auto toRead = CD3DX12_RESOURCE_BARRIER::Transition(resource_.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmdList->ResourceBarrier(1, &toRead);

	everUploaded_ = true;
}