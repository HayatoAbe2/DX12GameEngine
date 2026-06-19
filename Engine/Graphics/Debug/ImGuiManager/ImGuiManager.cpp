#include "ImGuiManager.h"

void ImGuiManager::Initialize([[maybe_unused]] HWND* hwnd, [[maybe_unused]] ID3D12Device* device, [[maybe_unused]] int bufferCount, [[maybe_unused]] DXGI_FORMAT format,
	[[maybe_unused]] ID3D12DescriptorHeap* srvHeap, [[maybe_unused]] D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle,
	[[maybe_unused]] D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle, [[maybe_unused]] ID3D12CommandQueue* cmdQueue) {
#ifdef USE_IMGUI
	// Imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui_ImplWin32_Init(*hwnd);

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.CommandQueue = cmdQueue;
	initInfo.Device = device;
	initInfo.NumFramesInFlight = bufferCount;
	initInfo.RTVFormat = format;
	initInfo.DSVFormat = format;
	initInfo.SrvDescriptorHeap = srvHeap;
	initInfo.LegacySingleSrvCpuDescriptor = srvCPUHandle;
	initInfo.LegacySingleSrvGpuDescriptor = srvGPUHandle;
	ImGui_ImplDX12_Init(&initInfo);

#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::BeginFrame() {
#ifdef USE_IMGUI
	// ImGuiフレーム
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Performance");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();
#endif
}

void ImGuiManager::EndFrame([[maybe_unused]] ID3D12GraphicsCommandList* cmdList) {
#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する
	ImGui::Render();

	// 実際のcommandListのImGuiの描画コマンドを読む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
#endif
}

void ImGuiManager::DrawSceneWindow(
	D3D12_GPU_DESCRIPTOR_HANDLE handle) {
#ifdef USE_IMGUI
	ImGui::Begin("Scene");

	ImVec2 size = ImGui::GetContentRegionAvail();

	ImGui::Image(
		(ImTextureID)handle.ptr,
		size
	);
	ImGui::End();
#endif
}