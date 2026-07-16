#include "ParticleCompute.h"

void ParticleCompute::Dispatch(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature, SRVManager* srvManager) {
}

void ParticleCompute::TransitionOutputBufferToUAV(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys) {
}

void ParticleCompute::TransitionOutputBufferToVB(ID3D12GraphicsCommandList* cmdList, ParticleSystem* particleSys) {
}
