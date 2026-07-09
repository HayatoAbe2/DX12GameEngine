#pragma once

#include "Engine/Graphics/Pipeline/BlendMode.h"
#include "Engine/Graphics/Renderer/PostEffectType.h"

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
class RootSignatureManager;
class PipelineStateManager {
public:

	void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, RootSignatureManager* rootSignatureManager);

	ID3D12PipelineState* GetStandardPSO(int index) { return standardPSO[index].Get(); }
	ID3D12PipelineState* GetInstancingPSO(int index) { return instancingPSO_[index].Get(); }
	ID3D12PipelineState* GetSpritePSO(int index) { return spritePSO_[index].Get(); }
	ID3D12PipelineState* GetParticlePSO(int index) { return particlePSO_[index].Get(); }
	ID3D12PipelineState* GetPrimitivePSO(int index) { return primitivePSO_[index].Get(); }
	ID3D12PipelineState* GetSkyboxPSO(int index) { return skyboxPSO_[index].Get(); }
	ID3D12PipelineState* GetGridPSO() { return gridPSO_.Get(); }
	ID3D12PipelineState* GetCopyImagePSO() { return copyImagePSO_.Get(); }
	ID3D12PipelineState* GetSceneViewPSO() { return sceneViewPSO_.Get(); }
	ID3D12PipelineState* GetPostEffectPSO(int effectType) { return postEffect[effectType].pso.Get(); }
	ID3D12PipelineState* GetSkinningComputePSO() { return skinningComputePSO_.Get(); }

	void SetStandardBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { standardPSOData.vertexShaderBlob = vsBlob; standardPSOData.pixelShaderBlob = psBlob; }
	void SetInstancingBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { instancingPSOData.vertexShaderBlob = vsBlob; instancingPSOData.pixelShaderBlob = psBlob; }
	void SetSpriteBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { spritePSOData.vertexShaderBlob = vsBlob; spritePSOData.pixelShaderBlob = psBlob; }
	void SetParticleBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { particlePSOData.vertexShaderBlob = vsBlob; particlePSOData.pixelShaderBlob = psBlob; }
	void SetPrimitiveBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { primitivePSOData.vertexShaderBlob = vsBlob; primitivePSOData.pixelShaderBlob = psBlob; }
	void SetSkyboxBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { skyboxPSOData.vertexShaderBlob = vsBlob; skyboxPSOData.pixelShaderBlob = psBlob; }
	void SetGridBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { gridPSOData.vertexShaderBlob = vsBlob; gridPSOData.pixelShaderBlob = psBlob; }
	
	void SetCopyImageBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { fullscreenPSOData.vertexShaderBlob = vsBlob; fullscreenPSOData.pixelShaderBlob = psBlob; }
	void SetSceneViewBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { sceneViewPSOData.vertexShaderBlob = vsBlob; sceneViewPSOData.pixelShaderBlob = psBlob; }
	void SetPostEffectPSBlob(int effectType, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { postEffect[effectType].psBlob = psBlob; }
	void SetSkinningComputeBlob(Microsoft::WRL::ComPtr<IDxcBlob> csBlob) { skinningComputePSOData.computeShaderBlob = csBlob; }
private:
	struct PSOData {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	};

	struct PostEffectData {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		Microsoft::WRL::ComPtr<IDxcBlob> psBlob = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	};

	struct ComputePSOData {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob;
	};

	PSOData standardPSOData;
	PSOData instancingPSOData;
	PSOData spritePSOData;
	PSOData particlePSOData;
	PSOData primitivePSOData;
	PSOData skyboxPSOData;
	PSOData gridPSOData;
	PSOData fullscreenPSOData;
	PSOData sceneViewPSOData;
	PostEffectData postEffect[int(PostEffectType::Count)];
	ComputePSOData skinningComputePSOData;

	void CreateStandardPSO();
	void CreateInstancingPSO();
	void CreateSpritePSO();
	void CreateParticlePSO();
	void CreatePrimitivePSO();
	void CreateSkyboxPSO();
	void CreateGridPSO();
	void CreateFullscreenPSO();
	void CreateSceneViewPSO();
	void CreatePostEffectPSO(PostEffectData& postEffect);
	void CreateSkinningComputePSO();

	// inputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[6] = {};
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
	Microsoft::WRL::ComPtr<ID3D12PipelineState> spritePSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> primitivePSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skyboxPSO_[6]{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> gridPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> sceneViewPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningComputePSO_ = nullptr;

	//
	// 参照
	//
	
	// デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

	D3D12_BLEND_DESC CreateNoneBlendDesc();
	D3D12_BLEND_DESC CreateAlphaBlendDesc();
	D3D12_BLEND_DESC CreateAddBlendDesc();
	D3D12_BLEND_DESC CreateSubtractBlendDesc();
	D3D12_BLEND_DESC CreateMultiplyBlendDesc();
	D3D12_BLEND_DESC CreateScreenBlendDesc();

	void CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc, const D3D12_BLEND_DESC& blendDesc, Microsoft::WRL::ComPtr<ID3D12PipelineState>* outPSO);

	// フルスクリーンDesc
	D3D12_GRAPHICS_PIPELINE_STATE_DESC fullscreenBaseDesc_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC sceneViewBaseDesc_;
};

