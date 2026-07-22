#include "PipelineStateManager.h"
#include "Engine/Graphics/Pipeline/RootSignatureManager/RootSignatureManager.h"
#include <cassert>

void PipelineStateManager::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, RootSignatureManager* rootSignatureManager) {
	device_ = device;
	mainPSOData[int(MainPSOType::Standard)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Standard);
	mainPSOData[int(MainPSOType::Instancing)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Instancing);
	mainPSOData[int(MainPSOType::Sprite)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Standard);
	mainPSOData[int(MainPSOType::Particle)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Particle);
	mainPSOData[int(MainPSOType::GPUParticle)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::GPUParticle);
	mainPSOData[int(MainPSOType::Primitive)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Standard);
	mainPSOData[int(MainPSOType::Skybox)].rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Skybox);
	gridPSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Grid);
	fullscreenPSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Fullscreen);
	sceneViewPSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::Fullscreen);
	skinningComputePSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::SkinningCompute);
	particleInitPSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::ParticleInit);
	particleEmitPSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::ParticleEmit);
	particleUpdatePSOData.rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::ParticleUpdate);

	// InputLayout
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[2].SemanticName = "NORMAL";
	inputElementDescs_[2].SemanticIndex = 0;
	inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[3].SemanticName = "COLOR";
	inputElementDescs_[3].SemanticIndex = 0;
	inputElementDescs_[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs_[4].SemanticName = "Weight";
	inputElementDescs_[4].SemanticIndex = 0;
	inputElementDescs_[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[4].InputSlot = 1;
	inputElementDescs_[4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[5].SemanticName = "Index";
	inputElementDescs_[5].SemanticIndex = 0;
	inputElementDescs_[5].Format = DXGI_FORMAT_R32G32B32A32_SINT;
	inputElementDescs_[5].InputSlot = 1;
	inputElementDescs_[5].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);


	CreateStandardPSO();
	CreateInstancingPSO();
	CreateSpritePSO();
	CreateParticlePSO();
	CreateGPUParticlePSO();
	CreatePrimitivePSO();
	CreateSkyboxPSO();
	CreateGridPSO();
	CreateFullscreenPSO();
	CreateSceneViewPSO();

	//
	// ポストエフェクト
	//

	for (int i = 1; i < int(PostEffectType::Count); ++i) {
		postEffect[i].desc = fullscreenBaseDesc_;
		CreatePostEffectPSO(postEffect[i]);
	}

	// スキニング
	CreateComputePSO(skinningComputePSOData, skinningComputePSO_);

	// パーティクル初期化
	CreateComputePSO(particleInitPSOData, particleInitPSO_);
	CreateComputePSO(particleEmitPSOData, particleEmitPSO_);
	CreateComputePSO(particleUpdatePSOData, particleUpdatePSO_);
}

void PipelineStateManager::CreateStandardPSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Standard)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	// 共通部分作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	baseDesc.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencil
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- 各ブレンドモードごとのPSO生成 ---
	auto& pso = mainPSO_[int(MainPSOType::Standard)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);			// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateInstancingPSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Instancing)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(),	psoData.pixelShaderBlob->GetBufferSize() };
	baseDesc.BlendState = CreateNoneBlendDesc();
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	auto& pso = mainPSO_[int(MainPSOType::Instancing)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);			// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateSpritePSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Sprite)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	// 共通部分作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	baseDesc.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencil
	baseDesc.DepthStencilState.DepthEnable = FALSE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- 各ブレンドモードごとのPSO生成 ---
	auto& pso = mainPSO_[int(MainPSOType::Sprite)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);			// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateParticlePSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Particle)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };
	baseDesc.BlendState = CreateNoneBlendDesc();
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	auto& pso = mainPSO_[int(MainPSOType::Particle)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateGPUParticlePSO() {
	auto& psoData = mainPSOData[int(MainPSOType::GPUParticle)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };
	baseDesc.BlendState = CreateNoneBlendDesc();
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	auto& pso = mainPSO_[int(MainPSOType::GPUParticle)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreatePrimitivePSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Primitive)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	// 共通部分作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	baseDesc.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencil
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// --- 各ブレンドモードごとのPSO生成 ---
	auto& pso = mainPSO_[int(MainPSOType::Primitive)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);			// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateSkyboxPSO() {
	auto& psoData = mainPSOData[int(MainPSOType::Skybox)];
	assert(psoData.rootSignature);
	assert(psoData.vertexShaderBlob);
	assert(psoData.pixelShaderBlob);

	// Skybox InputLayout
	D3D12_INPUT_ELEMENT_DESC skyboxInputElements[2]{};

	skyboxInputElements[0].SemanticName = "POSITION";
	skyboxInputElements[0].SemanticIndex = 0;
	skyboxInputElements[0].Format =
		DXGI_FORMAT_R32G32B32A32_FLOAT;
	skyboxInputElements[0].AlignedByteOffset =
		D3D12_APPEND_ALIGNED_ELEMENT;

	skyboxInputElements[1].SemanticName = "TEXCOORD";
	skyboxInputElements[1].SemanticIndex = 0;
	skyboxInputElements[1].Format =
		DXGI_FORMAT_R32G32B32_FLOAT;
	skyboxInputElements[1].AlignedByteOffset =
		D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC skyboxInputLayout{};
	skyboxInputLayout.pInputElementDescs =
		skyboxInputElements;
	skyboxInputLayout.NumElements =
		_countof(skyboxInputElements);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = psoData.rootSignature.Get();
	baseDesc.InputLayout = skyboxInputLayout;
	baseDesc.VS = { psoData.vertexShaderBlob->GetBufferPointer(), psoData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { psoData.pixelShaderBlob->GetBufferPointer(), psoData.pixelShaderBlob->GetBufferSize() };
	baseDesc.BlendState = CreateNoneBlendDesc();
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	auto& pso = mainPSO_[int(MainPSOType::Skybox)];
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &pso[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &pso[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &pso[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &pso[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &pso[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &pso[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateGridPSO() {
	assert(gridPSOData.rootSignature);
	assert(gridPSOData.vertexShaderBlob);
	assert(gridPSOData.pixelShaderBlob);

	// grid InputLayout
	D3D12_INPUT_ELEMENT_DESC gridInputElements[2]{};

	gridInputElements[0].SemanticName = "POSITION";
	gridInputElements[0].SemanticIndex = 0;
	gridInputElements[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	gridInputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	gridInputElements[1].SemanticName = "COLOR";
	gridInputElements[1].SemanticIndex = 0;
	gridInputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	gridInputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC gridInputLayout{};
	gridInputLayout.pInputElementDescs = gridInputElements;
	gridInputLayout.NumElements = _countof(gridInputElements);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = gridPSOData.rootSignature.Get();
	baseDesc.InputLayout = gridInputLayout;
	baseDesc.VS = { gridPSOData.vertexShaderBlob->GetBufferPointer(), gridPSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { gridPSOData.pixelShaderBlob->GetBufferPointer(), gridPSOData.pixelShaderBlob->GetBufferSize() };
	baseDesc.BlendState = CreateNoneBlendDesc();
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	baseDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	baseDesc.DepthStencilState.DepthEnable = TRUE;
	baseDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	baseDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	baseDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &gridPSO_);
}

void PipelineStateManager::CreateFullscreenPSO() {
	assert(fullscreenPSOData.rootSignature);
	assert(fullscreenPSOData.vertexShaderBlob);
	assert(fullscreenPSOData.pixelShaderBlob);

	// 共通部分作成
	fullscreenBaseDesc_.pRootSignature = fullscreenPSOData.rootSignature.Get();
	fullscreenBaseDesc_.InputLayout = {nullptr, 0}; // 使用しない
	fullscreenBaseDesc_.VS = { fullscreenPSOData.vertexShaderBlob->GetBufferPointer(), fullscreenPSOData.vertexShaderBlob->GetBufferSize() };
	fullscreenBaseDesc_.PS = { fullscreenPSOData.pixelShaderBlob->GetBufferPointer(), fullscreenPSOData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	fullscreenBaseDesc_.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	fullscreenBaseDesc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	fullscreenBaseDesc_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencil
	fullscreenBaseDesc_.DepthStencilState.DepthEnable = FALSE;

	fullscreenBaseDesc_.DSVFormat = DXGI_FORMAT_UNKNOWN;
	fullscreenBaseDesc_.NumRenderTargets = 1;
	fullscreenBaseDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	fullscreenBaseDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	fullscreenBaseDesc_.SampleDesc.Count = 1;
	fullscreenBaseDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	CreatePSO(fullscreenBaseDesc_, CreateNoneBlendDesc(), &copyImagePSO_); // ブレンドなし
}

void PipelineStateManager::CreateSceneViewPSO() {
	assert(sceneViewPSOData.rootSignature);
	assert(sceneViewPSOData.vertexShaderBlob);
	assert(sceneViewPSOData.pixelShaderBlob);

	// 共通部分作成
	sceneViewBaseDesc_.pRootSignature = sceneViewPSOData.rootSignature.Get();
	sceneViewBaseDesc_.InputLayout = { nullptr, 0 }; // 使用しない
	sceneViewBaseDesc_.VS = { sceneViewPSOData.vertexShaderBlob->GetBufferPointer(), sceneViewPSOData.vertexShaderBlob->GetBufferSize() };
	sceneViewBaseDesc_.PS = { sceneViewPSOData.pixelShaderBlob->GetBufferPointer(), sceneViewPSOData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	sceneViewBaseDesc_.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	sceneViewBaseDesc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	sceneViewBaseDesc_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencil
	sceneViewBaseDesc_.DepthStencilState.DepthEnable = FALSE;

	sceneViewBaseDesc_.DSVFormat = DXGI_FORMAT_UNKNOWN;
	sceneViewBaseDesc_.NumRenderTargets = 1;
	sceneViewBaseDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	sceneViewBaseDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	sceneViewBaseDesc_.SampleDesc.Count = 1;
	sceneViewBaseDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	CreatePSO(sceneViewBaseDesc_, CreateNoneBlendDesc(), &sceneViewPSO_); // ブレンドなし
}

void PipelineStateManager::CreatePostEffectPSO(PostEffectData& postEffect) {
	assert(fullscreenPSOData.rootSignature);
	assert(fullscreenPSOData.vertexShaderBlob);

	postEffect.desc.PS = { postEffect.psBlob->GetBufferPointer(), postEffect.psBlob->GetBufferSize() };

	CreatePSO(postEffect.desc, CreateNoneBlendDesc(), &postEffect.pso); // ブレンドなし
}

void PipelineStateManager::CreateComputePSO(ComputePSOData& psoData, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pso) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = psoData.rootSignature.Get();
	desc.CS = { psoData.computeShaderBlob->GetBufferPointer(), psoData.computeShaderBlob->GetBufferSize() };

	device_->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso));
}

// ----------------------------------------------------
// 各ブレンド設定生成
// ----------------------------------------------------

D3D12_BLEND_DESC PipelineStateManager::CreateNoneBlendDesc() {
	D3D12_BLEND_DESC desc{};
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return desc;
}

D3D12_BLEND_DESC PipelineStateManager::CreateAlphaBlendDesc() {
	D3D12_BLEND_DESC desc{};
	auto& rt = desc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rt.BlendOp = D3D12_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return desc;
}

D3D12_BLEND_DESC PipelineStateManager::CreateAddBlendDesc() {
	D3D12_BLEND_DESC desc{};
	auto& rt = desc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D12_BLEND_ONE;
	rt.BlendOp = D3D12_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return desc;
}

D3D12_BLEND_DESC PipelineStateManager::CreateSubtractBlendDesc() {
	D3D12_BLEND_DESC desc{};
	auto& rt = desc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D12_BLEND_ONE;
	rt.BlendOp = D3D12_BLEND_OP_SUBTRACT;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return desc;
}

D3D12_BLEND_DESC PipelineStateManager::CreateMultiplyBlendDesc() {
	D3D12_BLEND_DESC desc{};
	auto& rt = desc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D12_BLEND_ZERO;
	rt.DestBlend = D3D12_BLEND_SRC_COLOR;
	rt.BlendOp = D3D12_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return desc;
}

D3D12_BLEND_DESC PipelineStateManager::CreateScreenBlendDesc() {
	D3D12_BLEND_DESC desc{};
	auto& rt = desc.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	rt.DestBlend = D3D12_BLEND_ONE;
	rt.BlendOp = D3D12_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return desc;
}

// ----------------------------------------------------
// PSO生成
// ----------------------------------------------------

void PipelineStateManager::CreatePSO(
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
	const D3D12_BLEND_DESC& blendDesc,
	Microsoft::WRL::ComPtr<ID3D12PipelineState>* outPSO) {

	baseDesc.BlendState = blendDesc;
	HRESULT hr = device_->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(outPSO->ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));
}