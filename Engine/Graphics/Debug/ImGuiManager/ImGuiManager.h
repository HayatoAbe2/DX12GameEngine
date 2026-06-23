#pragma once
#include <format>
#include <d3d12.h>
#include <dxcapi.h>
#include <mfobjects.h>

#ifdef USE_IMGUI
#include "externals/DirectXTex/d3dx12.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "Engine/Editor/Scene/SceneEditor/GizmoCtx.h"
#endif
#include <Engine/Math/Vector2/Vector2.h>

class ImGuiManager {
public:
	void Initialize(HWND* hwnd, ID3D12Device* device, int bufferCount, DXGI_FORMAT format, ID3D12DescriptorHeap* srvHeap, D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle, ID3D12CommandQueue* cmdQueue);
	void Finalize();

	void BeginFrame();
	void EndFrame(ID3D12GraphicsCommandList* cmdList);

	void DrawSceneWindow(D3D12_GPU_DESCRIPTOR_HANDLE handle, RECT windowRect);

	Vector2 GetSceneWindowSize() { return sceneWindowSize_; }
#ifdef USE_IMGUI
	void SetGizmoCtx(const GizmoCtx& ctx) { gizmoCtx_ = ctx; }
	bool IsSceneHovered() { return isSceneHovered_; }
#endif
private:
	Vector2 sceneWindowSize_{};
#ifdef USE_IMGUI
	GizmoCtx gizmoCtx_{};
	bool isSceneHovered_ = false;
#endif
};
