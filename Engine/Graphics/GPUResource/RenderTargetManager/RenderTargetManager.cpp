#include "RenderTargetManager.h"
#include "Engine/Graphics/Core/DescriptorHeapManager/DescriptorHeapManager.h"
#include <cassert>

void RenderTargetManager::Initialize(IDXGISwapChain4* swapChain, ID3D12Device* device, DescriptorHeapManager* heapManager, std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>& renderTextureResource, const Microsoft::WRL::ComPtr<ID3D12Resource>& sceneViewResource) {
	// ---------------------
	// SwapChain
	// ---------------------

	for (int i = 0; i < 2; ++i) {
		HRESULT hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hr));
	}

	// RTVの設定
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2dテクスチャとして書き込む
	// ディスクリプタの先頭を取得する
	descriptorHandle_ = heapManager->GetCPUDescriptorHandle(heapManager->GetRTVHeap().Get(), heapManager->GetRTVHeapSize(), 0);
	// RTVを2つ作るのでディスクリプタ2つ
	rtvHandles_[0] = descriptorHandle_;
	device->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc_, rtvHandles_[0]);
	rtvHandles_[1].ptr = descriptorHandle_.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc_, rtvHandles_[1]);

	// ---------------------
	// RenderTexture
	// ---------------------

	CreateRTV(renderTextureRTVHandle_[0], device, renderTextureResource[0]);
	CreateRTV(renderTextureRTVHandle_[1], device, renderTextureResource[1]);
	CreateRTV(sceneViewRTVHandle_, device, sceneViewResource);
}

void RenderTargetManager::CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE& nextHandle, ID3D12Device* device, Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
	nextHandle.ptr = descriptorHandle_.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(resource.Get(), &rtvDesc_, nextHandle);
}

void RenderTargetManager::ReleaseSwapChainBuffers() {
	for (int i = 0; i < 2; i++) {
		swapChainResources_[i].Reset();
	}
}

void RenderTargetManager::Resize(
	IDXGISwapChain4* swapChain,
	ID3D12Device* device,
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> renderTexture,
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneView) {
	for (int i = 0; i < 2; i++) {
		swapChainResources_[i].Reset();
	}

	for (int i = 0; i < 2; i++) {
		HRESULT hr =
			swapChain->GetBuffer(
				i,
				IID_PPV_ARGS(&swapChainResources_[i]));

		assert(SUCCEEDED(hr));
	}

	device->CreateRenderTargetView(
		swapChainResources_[0].Get(),
		&rtvDesc_,
		rtvHandles_[0]);

	device->CreateRenderTargetView(
		swapChainResources_[1].Get(),
		&rtvDesc_,
		rtvHandles_[1]);

	for (int i = 0; i < 2; ++i) {
		device->CreateRenderTargetView(
			renderTexture[i].Get(),
			&rtvDesc_,
			renderTextureRTVHandle_[i]);
	}

	device->CreateRenderTargetView(
		sceneView.Get(),
		&rtvDesc_,
		sceneViewRTVHandle_);
}