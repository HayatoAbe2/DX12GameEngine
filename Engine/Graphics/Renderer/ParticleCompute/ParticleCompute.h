#pragma once
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h"
#include <d3d12.h>

class ParticleCompute {
public:
	void Emit(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* descriptorManager, Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource);
	void Update(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* descriptorManager, Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource);
};

