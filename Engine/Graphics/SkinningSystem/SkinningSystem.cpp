#include "SkinningSystem.h"

void SkinningSystem::Dispatch(ID3D12GraphicsCommandList* cmdList, Model* model, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* srvManager) {
	SkinClusterRuntime skinCluster = model->GetSkinCluster();
	SkinClusterData data = model->GetData()->skinClusterData_;

	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	auto& meshData = model->GetData()->meshes;
	for (int i = 0; i < meshData.size(); ++i) {
		auto& subMeshData = meshData[i].subMeshes;
		auto& subMeshRuntime = model->GetMesh()[i].subMeshes;
		for (int j = 0; j < subMeshData.size(); ++j) {
			cmdList->SetComputeRootDescriptorTable(0, skinCluster.paletteSrvHandle.second);
			cmdList->SetComputeRootDescriptorTable(1, srvManager->GetGPUHandle(subMeshData[j].inputVertexSRVIndex));
			cmdList->SetComputeRootDescriptorTable(2, srvManager->GetGPUHandle(data.influenceSRVIndex));
			cmdList->SetComputeRootDescriptorTable(4, srvManager->GetGPUHandle(subMeshRuntime[j].outputVertexUAVIndex));
			cmdList->SetComputeRootConstantBufferView(3, subMeshData[j].skinningInformationBuffer->GetGPUVirtualAddress());

			cmdList->Dispatch(UINT(subMeshData[j].vertices_.size() + 1023) / 1024, 1, 1);
		}
	}
}

void SkinningSystem::TransitionOutputBufferToUAV(ID3D12GraphicsCommandList* cmdList, Model* model) {
	auto& meshData = model->GetData()->meshes;
	for (int i = 0; i < meshData.size(); ++i) {
		auto& subMeshData = meshData[i].subMeshes;
		auto& subMeshRuntime = model->GetMesh()[i].subMeshes;
		for (int j = 0; j < subMeshData.size(); ++j) {
			if (!subMeshRuntime[j].outputVertexBuffer_) continue;

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = subMeshRuntime[j].outputVertexBuffer_.Get();
			barrier.Transition.StateBefore = subMeshRuntime[j].outputVertexBufferState;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			subMeshRuntime[j].outputVertexBufferState = barrier.Transition.StateAfter;

			cmdList->ResourceBarrier(1, &barrier);
		}
	}
}

void SkinningSystem::TransitionOutputBufferToVB(ID3D12GraphicsCommandList* cmdList, Model* model) {
	auto& meshData = model->GetData()->meshes;
	for (int i = 0; i < meshData.size(); ++i) {
		auto& subMeshData = meshData[i].subMeshes;
		auto& subMeshRuntime = model->GetMesh()[i].subMeshes;
		for (int j = 0; j < subMeshData.size(); ++j) {
			if (subMeshRuntime[j].outputVertexBuffer_ == nullptr) {
				continue;
			}

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = subMeshRuntime[j].outputVertexBuffer_.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			subMeshRuntime[j].outputVertexBufferState = barrier.Transition.StateAfter;

			cmdList->ResourceBarrier(1, &barrier);
		}
	}
}
