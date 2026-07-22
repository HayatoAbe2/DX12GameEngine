#pragma once

#include "Engine/Graphics/Pipeline/BlendMode.h"
#include "Engine/Graphics/Renderer/PostEffectType.h"

#include <array>
#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>

enum class MainPSOType {
	Standard,
	Instancing,
	Sprite,
	Particle,
	GPUParticle,
	Primitive,
	Skybox,

	Count
};

class RootSignatureManager;
class PipelineStateManager {
public:

	void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, RootSignatureManager* rootSignatureManager);

	ID3D12PipelineState* GetPSO(MainPSOType psoType, BlendMode mode) { return mainPSO_[int(psoType)][int(mode)].Get(); }

	ID3D12PipelineState* GetGridPSO() { return gridPSO_.Get(); }
	ID3D12PipelineState* GetCopyImagePSO() { return copyImagePSO_.Get(); }
	ID3D12PipelineState* GetSceneViewPSO() { return sceneViewPSO_.Get(); }
	ID3D12PipelineState* GetPostEffectPSO(int effectType) { return postEffect[effectType].pso.Get(); }
	ID3D12PipelineState* GetSkinningComputePSO() { return skinningComputePSO_.Get(); }
	ID3D12PipelineState* GetParticleInitPSO() { return particleInitPSO_.Get(); }
	ID3D12PipelineState* GetParticleEmitPSO() { return particleEmitPSO_.Get(); }
	ID3D12PipelineState* GetParticleUpdatePSO() { return particleUpdatePSO_.Get(); }

	void SetBlob(MainPSOType psoType, Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) {
		mainPSOData[static_cast<int>(psoType)].vertexShaderBlob = vsBlob;
		mainPSOData[static_cast<int>(psoType)].pixelShaderBlob = psBlob;
	}
	void SetGridBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { gridPSOData.vertexShaderBlob = vsBlob; gridPSOData.pixelShaderBlob = psBlob; }
	
	void SetCopyImageBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { fullscreenPSOData.vertexShaderBlob = vsBlob; fullscreenPSOData.pixelShaderBlob = psBlob; }
	void SetSceneViewBlob(Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { sceneViewPSOData.vertexShaderBlob = vsBlob; sceneViewPSOData.pixelShaderBlob = psBlob; }
	void SetPostEffectPSBlob(int effectType, Microsoft::WRL::ComPtr<IDxcBlob> psBlob) { postEffect[effectType].psBlob = psBlob; }

	void SetSkinningComputeBlob(Microsoft::WRL::ComPtr<IDxcBlob> csBlob) { skinningComputePSOData.computeShaderBlob = csBlob; }
	void SetParticleInitBlob(Microsoft::WRL::ComPtr<IDxcBlob> csBlob) { particleInitPSOData.computeShaderBlob = csBlob; }
	void SetParticleEmitBlob(Microsoft::WRL::ComPtr<IDxcBlob> csBlob) { particleEmitPSOData.computeShaderBlob = csBlob; }
	void SetParticleUpdateBlob(Microsoft::WRL::ComPtr<IDxcBlob> csBlob) { particleUpdatePSOData.computeShaderBlob = csBlob; }
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

	std::array<PSOData, size_t(MainPSOType::Count)> mainPSOData;
	PSOData gridPSOData;
	PSOData fullscreenPSOData;
	PSOData sceneViewPSOData;
	PostEffectData postEffect[int(PostEffectType::Count)];
	ComputePSOData skinningComputePSOData;
	ComputePSOData particleInitPSOData;
	ComputePSOData particleEmitPSOData;
	ComputePSOData particleUpdatePSOData;

	void CreateStandardPSO();
	void CreateInstancingPSO();
	void CreateSpritePSO();
	void CreateParticlePSO();
	void CreateGPUParticlePSO();
	void CreatePrimitivePSO();
	void CreateSkyboxPSO();
	void CreateGridPSO();
	void CreateFullscreenPSO();
	void CreateSceneViewPSO();
	void CreatePostEffectPSO(PostEffectData& postEffect);
	void CreateComputePSO(ComputePSOData& psoData, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pso);

	// inputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[(int)BlendMode::Count] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_;

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc_{};

	// ResterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc_ = {};

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// PSO
	std::array<std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, size_t(BlendMode::Count)>, size_t(MainPSOType::Count)> mainPSO_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> gridPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> sceneViewPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningComputePSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particleInitPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particleEmitPSO_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particleUpdatePSO_ = nullptr;

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

