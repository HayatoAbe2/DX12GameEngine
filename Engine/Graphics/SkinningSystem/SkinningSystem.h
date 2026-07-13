#pragma once
#include "Engine/SceneObject/Model/Model.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include <d3d12.h>

class SkinningSystem {
public:
	void Dispatch(ID3D12GraphicsCommandList* cmdList, Model* model, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* srvManager);

	// Dispatch前
	void TransitionOutputBufferToUAV(ID3D12GraphicsCommandList* cmdList, Model* model);

	// Dispatch後
	void TransitionOutputBufferToVB(ID3D12GraphicsCommandList* cmdList, Model* model);

private:
};

