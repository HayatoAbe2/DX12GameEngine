#include "PipelineStateManager.h"
#include "Engine/Graphics/Pipeline/RootSignatureManager/RootSignatureManager.h"
#include <cassert>

void PipelineStateManager::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, RootSignatureManager* rootSignatureManager) {
	device_ = device;
	standardPSOData.rootSignature = rootSignatureManager->GetStandardRootSignature();
	instancingPSOData.rootSignature = rootSignatureManager->GetInstancingRootSignature();
	spritePSOData.rootSignature = rootSignatureManager->GetStandardRootSignature();
	particlePSOData.rootSignature = rootSignatureManager->GetParticleRootSignature();
	skyboxPSOData.rootSignature = rootSignatureManager->GetSkyboxRootSignature();
	gridPSOData.rootSignature = rootSignatureManager->GetGridRootSignature();
	fullscreenPSOData.rootSignature = rootSignatureManager->GetFullscreenRootSignature();
	sceneViewPSOData.rootSignature = rootSignatureManager->GetFullscreenRootSignature();

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
	CreateSkyboxPSO();
	CreateGridPSO();
	CreateFullscreenPSO();
	CreateSceneViewPSO();

	//
	// ポストエフェクト
	//

	// grayscale
	postEffect[int(PostEffectType::Grayscale)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::Grayscale)]);

	// vignette
	postEffect[int(PostEffectType::Vignette)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::Vignette)]);

	// boxFilter3x3
	postEffect[int(PostEffectType::GaussianFilter3x3)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::GaussianFilter3x3)]);

	// boxFilter5x5
	postEffect[int(PostEffectType::BoxFilter5x5)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::BoxFilter5x5)]);

	// outline
	postEffect[int(PostEffectType::Outline)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::Outline)]);

	// radialBlur
	postEffect[int(PostEffectType::RadialBlur)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::RadialBlur)]);

	// dissolve
	postEffect[int(PostEffectType::Dissolve)].desc = fullscreenBaseDesc_;
	CreatePostEffectPSO(postEffect[int(PostEffectType::Dissolve)]);
}

void PipelineStateManager::CreateStandardPSO() {
	assert(standardPSOData.rootSignature);
	assert(standardPSOData.vertexShaderBlob);
	assert(standardPSOData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	// 共通部分作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = standardPSOData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { standardPSOData.vertexShaderBlob->GetBufferPointer(), standardPSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { standardPSOData.pixelShaderBlob->GetBufferPointer(), standardPSOData.pixelShaderBlob->GetBufferSize() };

	// ブレンド
	baseDesc.BlendState = CreateNoneBlendDesc();

	// ラスタライザ
	baseDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &standardPSO[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Normal)]);			// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateInstancingPSO() {
	assert(instancingPSOData.rootSignature);
	assert(instancingPSOData.vertexShaderBlob);
	assert(instancingPSOData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = instancingPSOData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { instancingPSOData.vertexShaderBlob->GetBufferPointer(), instancingPSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { instancingPSOData.pixelShaderBlob->GetBufferPointer(),	instancingPSOData.pixelShaderBlob->GetBufferSize() };
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

	CreatePSO(baseDesc, CreateNoneBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::Add)]);			// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &instancingPSO_[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateSpritePSO() {
	assert(spritePSOData.rootSignature);
	assert(spritePSOData.vertexShaderBlob);
	assert(spritePSOData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	// 共通部分作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = spritePSOData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { spritePSOData.vertexShaderBlob->GetBufferPointer(), spritePSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { spritePSOData.pixelShaderBlob->GetBufferPointer(), spritePSOData.pixelShaderBlob->GetBufferSize() };

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
	CreatePSO(baseDesc, CreateNoneBlendDesc(), &spritePSO[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &spritePSO[static_cast<int>(BlendMode::Normal)]);			// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &spritePSO[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &spritePSO[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &spritePSO[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &spritePSO[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateParticlePSO() {
	assert(particlePSOData.rootSignature);
	assert(particlePSOData.vertexShaderBlob);
	assert(particlePSOData.pixelShaderBlob);
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc{};
	baseDesc.pRootSignature = particlePSOData.rootSignature.Get();
	baseDesc.InputLayout = inputLayoutDesc_;
	baseDesc.VS = { particlePSOData.vertexShaderBlob->GetBufferPointer(), particlePSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { particlePSOData.pixelShaderBlob->GetBufferPointer(), particlePSOData.pixelShaderBlob->GetBufferSize() };
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

	CreatePSO(baseDesc, CreateNoneBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &particlePSO_[static_cast<int>(BlendMode::Screen)]);		// スクリーン
}

void PipelineStateManager::CreateSkyboxPSO() {
	assert(skyboxPSOData.rootSignature);
	assert(skyboxPSOData.vertexShaderBlob);
	assert(skyboxPSOData.pixelShaderBlob);

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
	baseDesc.pRootSignature = skyboxPSOData.rootSignature.Get();
	baseDesc.InputLayout = skyboxInputLayout;
	baseDesc.VS = { skyboxPSOData.vertexShaderBlob->GetBufferPointer(), skyboxPSOData.vertexShaderBlob->GetBufferSize() };
	baseDesc.PS = { skyboxPSOData.pixelShaderBlob->GetBufferPointer(), skyboxPSOData.pixelShaderBlob->GetBufferSize() };
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

	CreatePSO(baseDesc, CreateNoneBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::None)]);			// ブレンドなし
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::Normal)]);		// αブレンド
	CreatePSO(baseDesc, CreateAddBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::Add)]);				// 加算
	CreatePSO(baseDesc, CreateSubtractBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::Subtract)]);	// 減算
	CreatePSO(baseDesc, CreateMultiplyBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::Multiply)]);	// 乗算
	CreatePSO(baseDesc, CreateScreenBlendDesc(), &skyboxPSO_[static_cast<int>(BlendMode::Screen)]);		// スクリーン
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
