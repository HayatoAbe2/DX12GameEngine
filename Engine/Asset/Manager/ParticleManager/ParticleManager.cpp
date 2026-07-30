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
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIResource = bufferManager->CreateDefaultBuffer(sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource = bufferManager->CreateDefaultBuffer(sizeof(uint32_t) * size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// SRV
	uint32_t index = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferSRV(index, instanceTransformResource.Get(), UINT(size), sizeof(GPUParticle));
	particleSystem->SetInstanceResource(instanceTransformResource);
	particleSystem->SetSRVHandle(descriptorManager->GetGPUHandle(index));

	uint32_t emitterSize = (sizeof(EmitterSphere) + 255) & ~255;
	Microsoft::WRL::ComPtr<ID3D12Resource> emitterResource = bufferManager->CreateUploadBuffer(emitterSize);
	EmitterSphere* emitterData;
	emitterResource->Map(0, nullptr, reinterpret_cast<void**>(&emitterData));
	particleSystem->SetEmitterResource(emitterResource);
	particleSystem->SetEmitterData(emitterData);

	// UAV
	uint32_t uavIndex = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferUAV(uavIndex, instanceTransformResource.Get(), UINT(size), sizeof(GPUParticle));
	particleSystem->SetParticleUAVIndex(uavIndex);

	uavIndex = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferUAV(uavIndex, freeListIResource.Get(), 1, sizeof(int32_t));
	particleSystem->SetFreeCounterUAVIndex(uavIndex);
	particleSystem->SetFreeCounterResource(freeListIResource);

	uavIndex = descriptorManager->Allocate();
	descriptorManager->CreateStructuredBufferUAV(uavIndex, freeListResource.Get(), UINT(size), sizeof(uint32_t));
	particleSystem->SetFreeListUAVIndex(uavIndex);
	particleSystem->SetFreeListResource(freeListResource);

	// 初期化
	InitializeParticles(particleSystem.get(), size);
	particleSystem->InitializeGPUParticle();
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

	cmdList->SetComputeRootDescriptorTable(0, descriptorManager->GetGPUHandle(particleSys->GetParticleUAVIndex()));
	cmdList->SetComputeRootDescriptorTable(1, descriptorManager->GetGPUHandle(particleSys->GetFreeCounterUAVIndex()));
	cmdList->SetComputeRootDescriptorTable(2, descriptorManager->GetGPUHandle(particleSys->GetFreeListUAVIndex()));
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