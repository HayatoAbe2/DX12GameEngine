#include "PipelineStateManager.h"
#include <cassert>

void PipelineStateManager::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature,
	const Microsoft::WRL::ComPtr<ID3D12RootSignature>& instancingRootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& particleRootSignature,
	const Microsoft::WRL::ComPtr<ID3D12RootSignature>& skyboxRootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& copyImageRootSignature) {
	device_ = device;
	standardPSOData.rootSignature = rootSignature;
	instancingPSOData.rootSignature = instancingRootSignature;
	particlePSOData.rootSignature = particleRootSignature;
	skyboxPSOData.rootSignature = skyboxRootSignature;
	fullscreenPSOData.rootSignature = copyImageRootSignature;

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
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);


	CreateStandardPSO();
	CreateInstancingPSO();
	CreateParticlePSO();
	CreateSkyboxPSO();
	CreateFullscreenPSO();
	CreateGrayscalePSO();
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
	CreatePSO(baseDesc, CreateAlphaBlendDesc(), &standardPSO[static_cast<int>(BlendMode::Normal)]);		// αブレンド
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
	assert(inputLayoutDesc_.pInputElementDescs != nullptr);

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

	CreatePSO(fullscreenBaseDesc_, CreateNoneBlendDesc(), &copyImagePSO_[static_cast<int>(BlendMode::None)]); // ブレンドなし
}

void PipelineStateManager::CreateGrayscalePSO() {
	assert(fullscreenPSOData.rootSignature);
	assert(fullscreenPSOData.vertexShaderBlob);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = fullscreenBaseDesc_;
	desc.PS = { grayscalePSBlob_->GetBufferPointer(), grayscalePSBlob_->GetBufferSize() };

	CreatePSO(desc, CreateNoneBlendDesc(), &grayscalePSO_[static_cast<int>(BlendMode::None)]); // ブレンドなし
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
