#include "ShaderLibrary.h"
#include "Engine/Graphics/Pipeline/ShaderCompiler/ShaderCompiler.h"

#include <cassert>

void ShaderLibrary::Initialize(Logger* logger, ShaderCompiler* shaderCompiler, PipelineStateManager* pipelineStateManager) {
	logger_ = logger;
	shaderCompiler_ = shaderCompiler;
	pipelineStateManager_ = pipelineStateManager;
}

ShaderHandle ShaderLibrary::Register(const std::string& name, const ShaderDesc& desc) {
	// 登録済みなら再コンパイルせずにそのハンドルを返す
	auto it = nameToIndex_.find(name);
	if (it != nameToIndex_.end()) {
		return ShaderHandle{ it->second };
	}

	// コンパイル
	auto vsBlob = shaderCompiler_->Compile(desc.vs, L"vs_6_0", logger_);
	auto psBlob = shaderCompiler_->Compile(desc.ps, L"ps_6_0", logger_);

	// モデル用のPSO設定を流用してPSO作成
	ShaderData data;
	data.base = desc.base;
	pipelineStateManager_->CreateVariant(desc.base, vsBlob, psBlob, data.pso);

	shaders_.push_back(std::move(data));
	uint32_t index = uint32_t(shaders_.size() - 1);
	nameToIndex_[name] = index;
	return ShaderHandle{ index };
}

ShaderHandle ShaderLibrary::Find(const std::string& name) const {
	auto it = nameToIndex_.find(name);
	if (it == nameToIndex_.end()) {
		return ShaderHandle{};
	}
	return ShaderHandle{ it->second };
}

ID3D12PipelineState* ShaderLibrary::GetPSO(ShaderHandle handle, BlendMode blendMode) const {
	assert(handle.IsValid() && handle.index < shaders_.size());
	return shaders_[handle.index].pso[int(blendMode)].Get();
}

ID3D12RootSignature* ShaderLibrary::GetRootSignature(ShaderHandle handle) const {
	assert(handle.IsValid() && handle.index < shaders_.size());
	return pipelineStateManager_->GetRootSignature(shaders_[handle.index].base);
}

MainPSOType ShaderLibrary::GetBase(ShaderHandle handle) const {
	assert(handle.IsValid() && handle.index < shaders_.size());
	return shaders_[handle.index].base;
}
