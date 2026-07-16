#include "ParticleManager.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"

ParticleManager::ParticleManager(DirectXContext* dxContext) {
	dxContext_ = dxContext;
}

std::unique_ptr<ParticleSystem> ParticleManager::CreateParticle(uint32_t size, uint32_t id) {
	auto descriptorManager = dxContext_->GetSRVManager();
	auto bufferManager = dxContext_->GetBufferManager();
	
	std::unique_ptr<ParticleSystem> particleSystem = std::make_unique<ParticleSystem>(id);

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceTransformResource = bufferManager->CreateDefaultBuffer(sizeof(GPUParticle) * size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// SRV
	uint32_t index = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferSRV(index, instanceTransformResource.Get(), UINT(size), sizeof(GPUParticle));
	particleSystem->SetInstanceResource(instanceTransformResource);
	particleSystem->SetSRVHandle(descriptorManager->GetGPUHandle(index));

	// UAV
	uint32_t uavIndex = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferUAV(uavIndex, instanceTransformResource.Get(), UINT(size), sizeof(GPUParticle));
	particleSystem->SetUAVIndex(uavIndex);

	// 初期化
	InitializeParticles(particleSystem.get(), size);

	return std::move(particleSystem);
}

void ParticleManager::InitializeParticles(ParticleSystem* particleSys, uint32_t size) {
	auto commandListManager = dxContext_->GetCommandListManager();
	auto pipelineStateManager = dxContext_->GetPipelineStateManager();
	auto rootSignatureManager = dxContext_->GetRootSignatureManager();

	auto cmdList = commandListManager->GetCommandList();
	auto pipelineState = pipelineStateManager->GetParticleInitPSO();
	auto rootSignature = rootSignatureManager->GetRootSignature(RootSignatures::ParticleInit).Get();
	auto descriptorManager = dxContext_->GetSRVManager();

	// Barrier
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

	// Compute
	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	ID3D12DescriptorHeap* descriptorHeapsRaw[] = { descriptorManager->GetHeap().Get() };
	cmdList->SetDescriptorHeaps(1, descriptorHeapsRaw);

	cmdList->SetComputeRootDescriptorTable(0, descriptorManager->GetGPUHandle(particleSys->GetUAVIndex()));
	cmdList->Dispatch(UINT(size + 1023) / 1024, 1, 1);

	// Barrier
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