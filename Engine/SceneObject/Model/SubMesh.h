#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <vector>
#include "Engine/Graphics/GPUData/VertexData.h"
#include "Skeleton.h"

struct SkinningInformation {
	uint32_t numVertices;
};

struct SubMesh {
	std::vector<VertexData> vertices_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	int materialIndex_ = -1;

	std::vector<uint32_t> indices_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibv_{};

	// SkinCluster
	SkinCluster skinCluster_;

	Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW outputVBV_;

	uint32_t inputVertexSRVIndex;
	uint32_t outputVertexUAVIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> skinningInformationBuffer;

	D3D12_RESOURCE_STATES outputVertexBufferState = D3D12_RESOURCE_STATE_COMMON;
};
