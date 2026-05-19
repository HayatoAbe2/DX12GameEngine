#pragma once

#include "Engine/Graphics/Pipeline/BlendMode.h"

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
class PipelineStateManager {
public:

	void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& instancingRootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& particleRootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& skyboxRootSignature, const Microsoft::WRL::ComPtr<ID3D12RootSignature>& copyImageRootSignature);

	void CreateStandardPSO();
	void CreateInstancingPSO();
	void CreateSpritePSO();
	void CreateParticlePSO();
	void CreateSkyboxPSO();
	void CreateFullscreenPSO();
	void CreateGrayscalePSO();

	ID3D12PipelineState* GetStandardPSO(int index) { return standardPSO[index].Get(); }
	ID3D12PipelineState* GetInstancingPSO(int index) { return instancingPSO_[index].Get(); }
	ID3D12PipelineState* GetSpritePSO(int index) { return spritePSO[index].Get(); }
	ID3D12PipelineState* GetParticlePSO(int index) { return particlePSO_[index].Get(); }
	ID3D12PipelineState* GetSkyboxPSO(int index) { return skyboxPSO_[index].Get(); }
	ID3D12PipelineState* GetCopyImagePSO(int index) { return copyImagePSO_[index].Get(); }
	ID3D12PipelineState* GetGrayscalePSO(int index) { return grayscalePSO_[index].Get(); }

	void SetStandardBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { standardPSOData.vertexShaderBlob = vsBlob; standardPSOData.pixelShaderBlob = psBlob; }
	void SetInstancingBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { instancingPSOData.vertexShaderBlob = vsBlob; instancingPSOData.pixelShaderBlob = psBlob; }
	void SetSpriteBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { spritePSOData.vertexShaderBlob = vsBlob; spritePSOData.pixelShaderBlob = psBlob; }
	void SetParticleBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { particlePSOData.vertexShaderBlob = vsBlob; particlePSOData.pixelShaderBlob = psBlob; }
	void SetSkyboxBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { skyboxPSOData.vertexShaderBlob = vsBlob; skyboxPSOData.pixelShaderBlob = psBlob; }
	
	void SetCopyImageBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { fullscreenPSOData.vertexShaderBlob = vsBlob; fullscreenPSOData.pixelShaderBlob = psBlob; }
	void SetGrayScalePSBlob(Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { grayscalePSBlob_ = psBlob; }

private:

	// inputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[4] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_;

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc_{};

	// ResterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc_ = {};

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> standardPSO[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> instancingPSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> spritePSO[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skyboxPSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> grayscalePSO_[6]{};

	//
	// 参照
	//
	
	// デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

	struct PSOData {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	};
	PSOData standardPSOData;
	PSOData instancingPSOData;
	PSOData spritePSOData;
	PSOData particlePSOData;
	PSOData skyboxPSOData;
	PSOData fullscreenPSOData;
	PSOData grayscalePSOData;

	D3D12_BLEND_DESC CreateNoneBlendDesc();
	D3D12_BLEND_DESC CreateAlphaBlendDesc();
	D3D12_BLEND_DESC CreateAddBlendDesc();
	D3D12_BLEND_DESC CreateSubtractBlendDesc();
	D3D12_BLEND_DESC CreateMultiplyBlendDesc();
	D3D12_BLEND_DESC CreateScreenBlendDesc();

	void CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc, const D3D12_BLEND_DESC& blendDesc, Microsoft::WRL::ComPtr<ID3D12PipelineState>* outPSO);

	// フルスクリーンDesc
	D3D12_GRAPHICS_PIPELINE_STATE_DESC fullscreenBaseDesc_;
	Microsoft::WRL::ComPtr<IDxcBlob> grayscalePSBlob_ = nullptr;
};

