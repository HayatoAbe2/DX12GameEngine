#pragma once
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/SceneObject/Particle/ParticleSystem/ParticleSystem.h"
#include <d3d12.h>

class ParticleCompute {
public:
	void Dispatch(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* srvManager);

	// Dispatch前
	void TransitionOutputBufferToUAV(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys);

	// Dispatch後
	void TransitionOutputBufferToVB(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys);
};

