#include "SkinningSystem.h"

void SkinningSystem::Dispatch(ID3D12GraphicsCommandList* cmdList, Model* model, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* srvManager) {
	for (auto& mesh : model->GetData()->meshes) {
		for (auto& subMesh : mesh->GetPrimitives()) {
			cmdList->SetComputeRootSignature(rootSignature);
			cmdList->SetPipelineState(pipelineState);
			cmdList->SetComputeRootDescriptorTable(0, subMesh.skinCluster_.paletteSrvHandle.second);
			cmdList->SetComputeRootDescriptorTable(1, srvManager->GetGPUHandle(subMesh.inputVertexSRVIndex));
			cmdList->SetComputeRootDescriptorTable(2, srvManager->GetGPUHandle(subMesh.skinCluster_.influenceSRVIndex));
			cmdList->SetComputeRootDescriptorTable(4, srvManager->GetGPUHandle(subMesh.outputVertexUAVIndex));
			cmdList->SetComputeRootConstantBufferView(3, subMesh.skinningInformationBuffer->GetGPUVirtualAddress());

			cmdList->Dispatch(UINT(subMesh.vertices_.size() + 1023) / 1024, 1, 1);
		}
	}
}

void SkinningSystem::TransitionOutputBufferToUAV(ID3D12GraphicsCommandList* cmdList, Model* model) {
	for (auto& mesh : model->GetData()->meshes) {
		for (auto& subMesh : mesh->GetPrimitives()) {
			if (!subMesh.outputVertexBuffer_)
				continue;

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type =
				D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

			barrier.Transition.pResource =
				subMesh.outputVertexBuffer_.Get();

			barrier.Transition.StateBefore = subMesh.outputVertexBufferState;

			barrier.Transition.StateAfter =
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			subMesh.outputVertexBufferState = barrier.Transition.StateAfter;

			barrier.Transition.Subresource =
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(
				1,
				&barrier
			);
		}
	}
}

void SkinningSystem::TransitionOutputBufferToVB(ID3D12GraphicsCommandList* cmdList, Model* model) {
	for (auto& mesh : model->GetData()->meshes) {
		for (auto& subMesh : mesh->GetPrimitives()) {
			if (subMesh.outputVertexBuffer_ == nullptr) {
				continue;
			}

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type =
				D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

			barrier.Transition.pResource =
				subMesh.outputVertexBuffer_.Get();

			barrier.Transition.StateBefore =
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

			barrier.Transition.StateAfter =
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			subMesh.outputVertexBufferState = barrier.Transition.StateAfter;

			barrier.Transition.Subresource =
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(
				1,
				&barrier
			);
		}
	}
}
