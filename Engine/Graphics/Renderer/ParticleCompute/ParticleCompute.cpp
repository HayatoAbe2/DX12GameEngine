#include "ParticleCompute.h"

void ParticleCompute::Emit(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* descriptorManager, Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource) {
    cmdList->SetComputeRootSignature(rootSignature);
    cmdList->SetPipelineState(pipelineState);

	// TransitionBarrier
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleSys->GetInstanceResource().Get();
		barrier.Transition.StateBefore = particleSys->GetCurrentState();
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &barrier);
		particleSys->SetState(barrier.Transition.StateAfter);
	}

    cmdList->SetComputeRootConstantBufferView(0, particleSys->GetEmitterResource()->GetGPUVirtualAddress());
	cmdList->SetComputeRootConstantBufferView(1, perFrameResource->GetGPUVirtualAddress());
	cmdList->SetComputeRootDescriptorTable(2, descriptorManager->GetGPUHandle(particleSys->GetParticleUAVIndex()));
	cmdList->SetComputeRootDescriptorTable(3, descriptorManager->GetGPUHandle(particleSys->GetFreeCounterUAVIndex()));
	cmdList->Dispatch(1, 1, 1);
}

void ParticleCompute::Update(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* descriptorManager, Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource) {
	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	// UAVBarrier
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = particleSys->GetInstanceResource().Get();
		cmdList->ResourceBarrier(1, &barrier);
	}

	cmdList->SetComputeRootConstantBufferView(0, perFrameResource->GetGPUVirtualAddress());
	cmdList->SetComputeRootDescriptorTable(1, descriptorManager->GetGPUHandle(particleSys->GetParticleUAVIndex()));
	cmdList->Dispatch(1, 1, 1);

	// TransitionBarrier
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = particleSys->GetInstanceResource().Get();
		barrier.Transition.StateBefore = particleSys->GetCurrentState();
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		cmdList->ResourceBarrier(1, &barrier);
		particleSys->SetState(barrier.Transition.StateAfter);
	}
}