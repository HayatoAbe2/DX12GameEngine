#pragma once
#include "Engine/Io/Logger/Logger.h"
#include "Engine/Graphics/Core/DeviceManager/DeviceManager.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"
#include "Engine/Graphics/GPUResource/ConstantBufferManager/ConstantBufferManager.h"
#include "Engine/Graphics/Core/CommandListManager/CommandListManager.h"
#include "Engine/Graphics/Pipeline/RootSignatureManager/RootSignatureManager.h"
#include "Engine/Graphics/Pipeline/ShaderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/GPUResource/RenderTargetManager/RenderTargetManager.h"
#include "Engine/Graphics/Core/DescriptorHeapManager/DescriptorHeapManager.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/Graphics/Pipeline/PipelineStateManager/PipelineStateManager.h"
#include "Engine/Graphics/Utility/FixFPS/FixFPS.h"
#include "Engine/Graphics/Debug/ImGuiManager/ImGuiManager.h"
#include "Engine/Graphics/Renderer/PostEffectType.h"
#include "Engine/Scene/Camera/Camera.h"

#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <dxcapi.h>

#include <vector>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXContext {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(HWND hwnd, Logger* logger);

	/// <summary>
	/// 解放処理(ループ終了後に行う)
	/// </summary>
	void Finalize();

	/// <summary>
	/// フレーム開始時の処理(描画開始時に行う)
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了時の処理(描画終了時に行う)
	/// </summary>
	void EndFrame();

	// アクセサ
	RECT GetWindowRect() { return windowRect_; }
	DeviceManager* GetDeviceManager() { return deviceManager_.get(); }
	CommandListManager* GetCommandListManager() { return commandListManager_.get(); }
	DescriptorHeapManager* GetDescriptorHeapManager() { return descriptorHeapManager_.get(); }
	SRVManager* GetSRVManager() { return srvManager_.get(); }
	PipelineStateManager* GetPipelineStateManager() { return pipelineStateManager_.get(); }
	RootSignatureManager* GetRootSignatureManager() { return rootSignatureManager_.get(); }
	BufferManager* GetBufferManager() { return bufferManager_.get(); }
	ConstantBufferManager* GetConstantBufferManager() { return constantBufferManager_.get(); }
	FixFPS* GetFixFPS() { return fixFPS_.get(); }
	Vector2 GetSceneWindowSize() { return imGuiManager_->GetSceneWindowSize(); }

	void SetPostEffectType(PostEffectType type) { postEffectType_ = type; }
	void SetDissolveMask(D3D12_GPU_DESCRIPTOR_HANDLE handle) {dissolveMaskHandle_ = handle;}
	void SetCamera(Camera* camera) { camera_ = camera; }

#ifdef USE_IMGUI
	bool GetSceneWindowHovered() { return imGuiManager_->IsSceneHovered(); }
	void SetGizmoCtx(const GizmoCtx& ctx) { imGuiManager_->SetGizmoCtx(ctx); }
#endif
private:

	/// <summary>
	/// SwapChain初期化
	/// </summary>
	void InitializeSwapChain(HWND hwnd);

	/// <summary>
	/// RenderTextureの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource();

	/// <summary>
	/// SceneView作成
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateSceneViewResource();

	/// <summary>
	/// Viewport,Scissor設定
	/// </summary>
	void SetViewportAndScissor();

	/// <summary>
	/// TextureResource作成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device);

	// DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

	// デバイスマネージャー
	std::unique_ptr<DeviceManager> deviceManager_ = nullptr;

	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	UINT backBufferIndex_;

	// 深度ステンシルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	uint32_t depthTextureSRVIndex_;

	// RenderTextureリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_ = nullptr;
	uint32_t renderTextureSRVIndex_;

	// 遷移バリア
	D3D12_RESOURCE_BARRIER swapChainBarrier_ = {};
	D3D12_RESOURCE_BARRIER renderTextureBarrier_ = {};
	D3D12_RESOURCE_BARRIER sceneViewBarrier_ = {};
	D3D12_RESOURCE_BARRIER depthBarrier_ = {};

	// ビューポート
	D3D12_VIEWPORT viewport_ = {};

	// シザー矩形
	D3D12_RECT scissorRect_ = {};

	// ログクラス
	Logger* logger_;

	// シェーダーコンパイルクラス
	std::unique_ptr<ShaderCompiler> shaderCompiler_ = nullptr;

	// ディスクリプタヒープ管理クラス
	std::unique_ptr<DescriptorHeapManager> descriptorHeapManager_ = nullptr;

	// RTV設定など
	std::unique_ptr<RenderTargetManager> renderTargetManager_ = nullptr;

	// SRVマネージャー
	std::unique_ptr<SRVManager> srvManager_ = nullptr;

	// コマンド関連
	std::unique_ptr<CommandListManager> commandListManager_ = nullptr;

	// ルートシグネチャ管理クラス
	std::unique_ptr<RootSignatureManager> rootSignatureManager_ = nullptr;

	// パイプラインステート管理クラス
	std::unique_ptr<PipelineStateManager> pipelineStateManager_ = nullptr;

	// FPS固定クラス
	std::unique_ptr<FixFPS> fixFPS_ = nullptr;

	// ImGui管理クラス
	std::unique_ptr<ImGuiManager> imGuiManager_ = nullptr;

	// バッファ管理クラス
	std::unique_ptr<BufferManager> bufferManager_ = nullptr;
	std::unique_ptr<ConstantBufferManager> constantBufferManager_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	PostEffectType postEffectType_ = PostEffectType::None;

	struct OutlineData {
		Matrix4x4 projectionInverse;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> outlineResource_;
	OutlineData* outlineData_ = nullptr;

	// 前フレームのウィンドウサイズ
	HWND hwnd_{};
	RECT windowRect_{};

	// デバッグでの、シーン描画用
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneViewResource_;
	uint32_t sceneViewSRVIndex_;

	uint32_t sceneViewRTVIndex_;
	D3D12_CPU_DESCRIPTOR_HANDLE sceneViewRTVHandle_;


	D3D12_GPU_DESCRIPTOR_HANDLE dissolveMaskHandle_{};

};

