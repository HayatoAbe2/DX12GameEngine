#include "DirectXContext.h"
#include <Windows.h>

#include <cassert>
#include <format>
#include <dxcapi.h>
#include <mfobjects.h>
#include <numbers>

void DirectXContext::Initialize(HWND hwnd, Logger* logger) {
	HRESULT hr;
	hwnd_ = hwnd;

	// ウィンドウサイズ
	GetClientRect(hwnd, &windowRect_);

	logger_ = logger;

	// FPS固定クラス初期化
	fixFPS_ = std::make_unique<FixFPS>();
	fixFPS_->Initialize();

	// DXGIファクトリーの生成
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr));

	// デバイスマネージャー初期化
	deviceManager_ = std::make_unique<DeviceManager>();
	deviceManager_->Initialize(dxgiFactory_.Get(), logger_);

	// バッファ管理クラス
	bufferManager_ = std::make_unique<BufferManager>(deviceManager_->GetDevice().Get());
	constantBufferManager_ = std::make_unique<ConstantBufferManager>(deviceManager_->GetDevice().Get());

	// コマンドリストマネージャー初期化
	commandListManager_ = std::make_unique<CommandListManager>();
	commandListManager_->Initialize(deviceManager_.get());

	// スワップチェーンの生成
	InitializeSwapChain(hwnd);

	// ディスクリプタヒープの初期化
	descriptorHeapManager_ = std::make_unique<DescriptorHeapManager>();
	descriptorHeapManager_->Initialize(deviceManager_->GetDevice().Get());

	// RenderTexture
	renderTextureResource_ = CreateRenderTextureResource();

	// RTV作成
	renderTargetManager_ = std::make_unique<RenderTargetManager>();
	renderTargetManager_->Initialize(swapChain_.Get(), deviceManager_->GetDevice().Get(), descriptorHeapManager_.get(), renderTextureResource_);

	// SRVマネージャー
	srvManager_ = std::make_unique<SRVManager>();
	srvManager_->Initialize(descriptorHeapManager_.get(), deviceManager_->GetDevice().Get());

	renderTextureSRVIndex_ = srvManager_->Allocate();
	srvManager_->CreateRenderTextureSRV(renderTextureSRVIndex_, renderTextureResource_);
	srvManager_->GetGPUHandle(renderTextureSRVIndex_);

	// DepthStencilTextureをウィンドウのサイズで作成
	depthStencilResource_ = CreateDepthStencilTextureResource(deviceManager_->GetDevice());
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture
	// DSVHeapの先頭にDSVを作る
	deviceManager_->GetDevice()->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, descriptorHeapManager_->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());
	// DepthのSRV
	depthTextureSRVIndex_ = srvManager_->Allocate();
	srvManager_->CreateDepthSRV(depthTextureSRVIndex_, depthStencilResource_);

	// ルートシグネチャマネージャー
	rootSignatureManager_ = std::make_unique<RootSignatureManager>();
	rootSignatureManager_->Initialize(deviceManager_->GetDevice(), logger_);

	shaderCompiler_ = std::make_unique<ShaderCompiler>();
	shaderCompiler_->Initialize();

	// Shaderをコンパイルする
	pipelineStateManager_ = std::make_unique<PipelineStateManager>();
	pipelineStateManager_->SetStandardBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Object3D.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Object3D.PS.hlsl", L"ps_6_0", logger_)
	);
	pipelineStateManager_->SetInstancingBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Instance.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Instance.PS.hlsl", L"ps_6_0", logger_)
	);
	pipelineStateManager_->SetSpriteBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Object3D.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Object3D.PS.hlsl", L"ps_6_0", logger_)
	);
	pipelineStateManager_->SetParticleBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Particle.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Particle.PS.hlsl", L"ps_6_0", logger_)
	);
	pipelineStateManager_->SetSkyboxBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Skybox.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Skybox.PS.hlsl", L"ps_6_0", logger_)
	);
	pipelineStateManager_->SetGridBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Grid.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Grid.PS.hlsl", L"ps_6_0", logger_)
	);

	pipelineStateManager_->SetCopyImageBlob(
		shaderCompiler_->Compile(L"Resources/Shaders/Fullscreen.VS.hlsl", L"vs_6_0", logger_),
		shaderCompiler_->Compile(L"Resources/Shaders/Fullscreen.PS.hlsl", L"ps_6_0", logger_)
	);
	// ポストエフェクト
	pipelineStateManager_->SetPostEffectPSBlob(int(PostEffectType::Grayscale), shaderCompiler_->Compile(L"Resources/Shaders/Grayscale.PS.hlsl", L"ps_6_0", logger_));
	pipelineStateManager_->SetPostEffectPSBlob(int(PostEffectType::Vignette), shaderCompiler_->Compile(L"Resources/Shaders/Vignette.PS.hlsl", L"ps_6_0", logger_));
	pipelineStateManager_->SetPostEffectPSBlob(int(PostEffectType::GaussianFilter3x3), shaderCompiler_->Compile(L"Resources/Shaders/GaussianFilter3x3.PS.hlsl", L"ps_6_0", logger_));
	pipelineStateManager_->SetPostEffectPSBlob(int(PostEffectType::BoxFilter5x5), shaderCompiler_->Compile(L"Resources/Shaders/BoxFilter5x5.PS.hlsl", L"ps_6_0", logger_));
	pipelineStateManager_->SetPostEffectPSBlob(int(PostEffectType::Outline), shaderCompiler_->Compile(L"Resources/Shaders/Outline/Outline.PS.hlsl", L"ps_6_0", logger_));

	// Outline用リソース
	outlineResource_ = bufferManager_->CreateUploadBuffer(sizeof(OutlineData));
	outlineResource_->Map(0, nullptr, reinterpret_cast<void**>(&outlineData_));

	// PSOマネージャー
	pipelineStateManager_->Initialize(deviceManager_->GetDevice(), rootSignatureManager_.get());

	SetViewportAndScissor();

	imGuiManager_ = std::make_unique<ImGuiManager>();
	int index = srvManager_->Allocate();
	imGuiManager_->Initialize(&hwnd, deviceManager_->GetDevice().Get(),
		swapChainDesc_.BufferCount,
		renderTargetManager_->GetRTVDesc_().Format,
		srvManager_->GetHeap().Get(),
		srvManager_->GetCPUHandle(index),
		srvManager_->GetGPUHandle(index),
		commandListManager_->GetCommandQueue().Get()
	);
}

void DirectXContext::Finalize() {
	commandListManager_->Wait();

	descriptorHeapManager_.reset();
	deviceManager_.reset();

	if (rootSignatureManager_->GetErrorBlob()) rootSignatureManager_->GetErrorBlob()->Release();
	imGuiManager_->Finalize();
}

void DirectXContext::BeginFrame() {
	RECT newRect;
	GetClientRect(hwnd_, &newRect);
	// ウィンドウサイズ変更時の更新
	if ((newRect.left != windowRect_.left ||
		newRect.right != windowRect_.right ||
		newRect.top != windowRect_.top ||
		newRect.bottom != windowRect_.bottom) &&
		newRect.right > 0 &&
		newRect.bottom > 0) {
		windowRect_ = newRect;
		commandListManager_->GetCommandList()->Close();

		SetViewportAndScissor();
		commandListManager_->Wait();
		commandListManager_->Reset();
		renderTargetManager_->ReleaseSwapChainBuffers();
		renderTextureResource_.Reset();
		depthStencilResource_.Reset();
		swapChain_->ResizeBuffers(2, UINT(windowRect_.right), UINT(windowRect_.bottom), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		// RenderTexture再生成
		renderTextureResource_ = CreateRenderTextureResource();
		
		// DepthSRV再作成
		depthStencilResource_ = CreateDepthStencilTextureResource(deviceManager_->GetDevice());

		// RTV再生成
		renderTargetManager_->Resize(swapChain_.Get(), deviceManager_->GetDevice().Get(), renderTextureResource_.Get());

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture
		deviceManager_->GetDevice()->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, descriptorHeapManager_->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());

		// SRV再作成
		srvManager_->CreateRenderTextureSRV(renderTextureSRVIndex_, renderTextureResource_);
		srvManager_->CreateDepthSRV(depthTextureSRVIndex_, depthStencilResource_);
	}

	srvManager_->PreDraw(commandListManager_->GetCommandList());

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = descriptorHeapManager_->GetCPUDescriptorHandle(descriptorHeapManager_->GetDSVHeap().Get(), descriptorHeapManager_->GetDSVHeapSize(), 0);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTargetManager_->GetRenderTextureRTVHandle();
	commandListManager_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 青っぽい色。RGBAの順
	commandListManager_->GetCommandList()->ClearRenderTargetView(renderTargetManager_->GetRenderTextureRTVHandle(), clearColor, 0, nullptr);
	// 指定した深度で画面全体をクリアする
	commandListManager_->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// 描画用のDescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvManager_->GetHeap().Get() };
	commandListManager_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
	// Viewportを設定
	commandListManager_->GetCommandList()->RSSetViewports(1, &viewport_);
	// Scissorを設定
	commandListManager_->GetCommandList()->RSSetScissorRects(1, &scissorRect_);

	// CBV書き込み先リセット
	constantBufferManager_->BeginFrame();

	imGuiManager_->BeginFrame();

	imGuiManager_->DrawSceneWindow(srvManager_->GetGPUHandle(renderTextureSRVIndex_));
}

void DirectXContext::EndFrame() {
	auto cmdList = commandListManager_->GetCommandList();

	renderTextureBarrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderTextureBarrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderTextureBarrier_.Transition.pResource = renderTextureResource_.Get();
	renderTextureBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	renderTextureBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandListManager_->GetCommandList()->ResourceBarrier(1, &renderTextureBarrier_);

	// これから書き込むバックバッファのインデックスを取得
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	// TransitionBarrierの設定
	// バリアはTransition
	swapChainBarrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	swapChainBarrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現存のバックバッファに対して行う
	swapChainBarrier_.Transition.pResource = renderTargetManager_->GetSwapChainResource(backBufferIndex_).Get();
	// 遷移前(現存)のResourceState
	swapChainBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	swapChainBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	assert(renderTargetManager_->GetSwapChainResource(backBufferIndex_));

	// depthBarrier
	if (postEffectType_ == PostEffectType::Outline) {
		depthBarrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		depthBarrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		depthBarrier_.Transition.pResource = depthStencilResource_.Get();
		depthBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		depthBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		cmdList->ResourceBarrier(1, &depthBarrier_);
	}

	cmdList->ResourceBarrier(1, &swapChainBarrier_);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = descriptorHeapManager_->GetCPUDescriptorHandle(descriptorHeapManager_->GetDSVHeap().Get(), descriptorHeapManager_->GetDSVHeapSize(), 0);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTargetManager_->GetRTVHandle(backBufferIndex_);
	cmdList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 青っぽい色。RGBAの順
	cmdList->ClearRenderTargetView(renderTargetManager_->GetRTVHandle(backBufferIndex_), clearColor, 0, nullptr);
	// 描画用のDescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvManager_->GetHeap().Get() };
	cmdList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
	// Viewportを設定
	cmdList->RSSetViewports(1, &viewport_);
	// Scissorを設定
	cmdList->RSSetScissorRects(1, &scissorRect_);

	// コピー
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	switch (postEffectType_) {
	case PostEffectType::Grayscale:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetPostEffectPSO(int(PostEffectType::Grayscale)));

		break;
	case PostEffectType::Vignette:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetPostEffectPSO(int(PostEffectType::Vignette)));

		break;
	case PostEffectType::GaussianFilter3x3:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetPostEffectPSO(int(PostEffectType::GaussianFilter3x3)));
		break;
	case PostEffectType::BoxFilter5x5:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetPostEffectPSO(int(PostEffectType::BoxFilter5x5)));
		break;
	case PostEffectType::Outline:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetPostEffectPSO(int(PostEffectType::Outline)));

		outlineData_->projectionInverse = Inverse(camera_->projectionMatrix_);
		cmdList->SetGraphicsRootConstantBufferView(2, outlineResource_->GetGPUVirtualAddress());
		break;
	default:
		cmdList->SetGraphicsRootSignature(rootSignatureManager_->GetFullscreenRootSignature().Get());
		cmdList->SetPipelineState(pipelineStateManager_->GetCopyImagePSO());
		break;
	}
	cmdList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUHandle(renderTextureSRVIndex_));
	cmdList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUHandle(depthTextureSRVIndex_));
	cmdList->DrawInstanced(3, 1, 0, 0);

	// depthBarrier
	if (postEffectType_ == PostEffectType::Outline) {
		depthBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		depthBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		cmdList->ResourceBarrier(1, &depthBarrier_);
	}

	imGuiManager_->EndFrame(commandListManager_->GetCommandList().Get());

	// 今回はRenderTargetからPresentにする
	swapChainBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	swapChainBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	commandListManager_->GetCommandList()->ResourceBarrier(1, &swapChainBarrier_);

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	HRESULT hr = commandListManager_->GetCommandList()->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandListManager_->GetCommandList().Get() };
	commandListManager_->GetCommandQueue()->ExecuteCommandLists(1, commandLists->GetAddressOf());

	// GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

	commandListManager_->Wait();
	commandListManager_->Reset();

	fixFPS_->Update();

	// 次フレーム、RenderTextureに対して描画
	renderTextureBarrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	renderTextureBarrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	renderTextureBarrier_.Transition.pResource = renderTextureResource_.Get();
	renderTextureBarrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	renderTextureBarrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandListManager_->GetCommandList()->ResourceBarrier(1, &renderTextureBarrier_);

}

void DirectXContext::InitializeSwapChain(HWND hwnd) {
	HRESULT hr;
	RECT rect;
	GetClientRect(hwnd, &rect);

	swapChainDesc_.Width = rect.right;								// 画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc_.Height = rect.bottom;							// 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// 色の形式
	swapChainDesc_.SampleDesc.Count = 1;							// マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画のターゲットとして利用する
	swapChainDesc_.BufferCount = 2;									// ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// モニタにうつしたら、中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandListManager_->GetCommandQueue().Get(), hwnd, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXContext::CreateRenderTextureResource() {
	HRESULT hr;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = windowRect_.right;
	resourceDesc.Height = windowRect_.bottom;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;	// VRAM上に作る

	// クリア設定
	D3D12_CLEAR_VALUE clearValue{};
	Vector4 clearColor = { 0.1f,0.25f,0.5f,1.0f };
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = deviceManager_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue, // Clear最適値
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXContext::CreateDepthStencilTextureResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device) {

	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = windowRect_.right;							// Textureの幅
	resourceDesc.Height = windowRect_.bottom;						// Textureの高さ
	resourceDesc.MipLevels = 1;										// mipmapの数
	resourceDesc.DepthOrArraySize = 1;								// 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			// DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;								// サンプル数(1固定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;	// VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;	// 1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// フォーマット。Resourceと合わせる

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,					// Heapの設定
		D3D12_HEAP_FLAG_NONE,				// Heapの特殊な設定。特になし。	
		&resourceDesc,						// Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// 深度値を書き込む状態にしておく
		&depthClearValue,					// Clear最適値
		IID_PPV_ARGS(&resource));			// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

void DirectXContext::SetViewportAndScissor() {
	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = float(windowRect_.right);
	viewport_.Height = float(windowRect_.bottom);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	// シザー矩形
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = windowRect_.right;
	scissorRect_.top = 0;
	scissorRect_.bottom = windowRect_.bottom;
}