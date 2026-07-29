#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <cstdint>
#include <array>

class DescriptorHeapManager;
class RenderTargetManager {
public:
	/// <summary>
	/// RTV作成
	/// </summary>
	/// <param name="swapChain">スワップチェーン</param>
	/// <param name="device">デバイス</param>
	/// <param name="heapManager">ヒープ管理クラスのインスタンス</param>x]
	/// <param name="renderTextureResource">RenderTextureのリソース</param>
	void Initialize(IDXGISwapChain4* swapChain, ID3D12Device* device, DescriptorHeapManager* heapManager, std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>& renderTextureResource, const Microsoft::WRL::ComPtr<ID3D12Resource>& sceneViewResource);
	void CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE& nextHandle, ID3D12Device* device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);

	void ReleaseSwapChainBuffers();
	void Resize(IDXGISwapChain4* swapChain, ID3D12Device* device, std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> renderTexture, Microsoft::WRL::ComPtr<ID3D12Resource> sceneView);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(size_t index) const { return rtvHandles_[index]; }
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> GetRenderTextureRTVHandle() const { return renderTextureRTVHandle_; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetPostEffectRTVHandle() const { return postEffectRTVHandle_; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSceneViewRTVHandle() const { return sceneViewRTVHandle_; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetSwapChainResource(size_t index) { return swapChainResources_[index]; }

	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc_() { return rtvDesc_; }

private:
	// RTVDesc
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

	// RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> renderTextureRTVHandle_;
	D3D12_CPU_DESCRIPTOR_HANDLE postEffectRTVHandle_;
	D3D12_CPU_DESCRIPTOR_HANDLE sceneViewRTVHandle_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle_;
};

