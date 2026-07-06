#include "ImGuiManager.h"
#include <Engine/Contexts/GameContext/GameContext.h>

Matrix4x4 ToMatrix4x4(const float m[16]) {
	Matrix4x4 r{};

	r.m[0][0] = m[0];
	r.m[0][1] = m[1];
	r.m[0][2] = m[2];
	r.m[0][3] = m[3];

	r.m[1][0] = m[4];
	r.m[1][1] = m[5];
	r.m[1][2] = m[6];
	r.m[1][3] = m[7];

	r.m[2][0] = m[8];
	r.m[2][1] = m[9];
	r.m[2][2] = m[10];
	r.m[2][3] = m[11];

	r.m[3][0] = m[12];
	r.m[3][1] = m[13];
	r.m[3][2] = m[14];
	r.m[3][3] = m[15];

	return r;
}

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

void ImGuiManager::DrawSceneWindow(D3D12_GPU_DESCRIPTOR_HANDLE handle, RECT windowRect) {
#ifdef USE_IMGUI
	ImGui::Begin("Scene");
	isSceneHovered_ = ImGui::IsWindowHovered();

	float textureWidth = float(windowRect.right);
	float textureHeight = float(windowRect.bottom);
	float aspect = textureWidth / textureHeight;

	ImVec2 avail = ImGui::GetContentRegionAvail();

	ImVec2 imageSize;

	if (avail.x / avail.y > aspect) {
		imageSize.y = avail.y;
		imageSize.x = avail.y * aspect;
	} else {
		imageSize.x = avail.x;
		imageSize.y = avail.x / aspect;
	}

	ImGui::Image(
		(ImTextureID)handle.ptr,
		imageSize
	);

	ImVec2 size = ImGui::GetItemRectSize();
	sceneWindowSize_.x = size.x;
	sceneWindowSize_.y = size.y;

	if (gizmoCtx_.isActive) {
		Vector2 winSize = GameContext::GetInstance().GetWindowSize();
		ImGuizmo::SetRect(0, 0, winSize.x, winSize.y);
		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate(
			gizmoCtx_.view,
			gizmoCtx_.proj,
			gizmoCtx_.op,
			ImGuizmo::WORLD,
			gizmoCtx_.modelMatrix,
			nullptr,
			gizmoCtx_.useSnap ? gizmoCtx_.snap : nullptr
		);

		if (ImGuizmo::IsUsing()) {
			if (auto* target = dynamic_cast<Model*>(gizmoCtx_.target)) {
				Matrix4x4 m = (ToMatrix4x4(gizmoCtx_.modelMatrix));
				Transform t = target->GetTransform();

				t.scale.x = Length(Vector3(m.m[0][0], m.m[0][1], m.m[0][2]));
				t.scale.y = Length(Vector3(m.m[1][0], m.m[1][1], m.m[1][2]));
				t.scale.z = Length(Vector3(m.m[2][0], m.m[2][1], m.m[2][2]));

				Matrix4x4 rot = m;
				rot.m[0][0] /= t.scale.x;
				rot.m[0][1] /= t.scale.x;
				rot.m[0][2] /= t.scale.x;
				rot.m[1][0] /= t.scale.y;
				rot.m[1][1] /= t.scale.y;
				rot.m[1][2] /= t.scale.y;
				rot.m[2][0] /= t.scale.z;
				rot.m[2][1] /= t.scale.z;
				rot.m[2][2] /= t.scale.z;
				t.rotate.y = asin(-rot.m[0][2]);
				t.rotate.x = atan2(rot.m[1][2], rot.m[2][2]);
				t.rotate.z = atan2(rot.m[0][1], rot.m[0][0]);
				t.translate.x = m.m[3][0];
				t.translate.y = m.m[3][1];
				t.translate.z = m.m[3][2];
				target->SetTransform(t);

			} else if (auto* target = dynamic_cast<InstancedModel*>(gizmoCtx_.target)) {
				Matrix4x4 m = (ToMatrix4x4(gizmoCtx_.modelMatrix));
				Transform t = target->GetTransforms()[gizmoCtx_.editingInstance];

				if (!wasUsing_) {
					beginTransform_ = target->GetTransforms()[gizmoCtx_.editingInstance];
				}

				// Matrix→Transform変換
				t.scale.x = Length(Vector3(m.m[0][0], m.m[0][1], m.m[0][2]));
				t.scale.y = Length(Vector3(m.m[1][0], m.m[1][1], m.m[1][2]));
				t.scale.z = Length(Vector3(m.m[2][0], m.m[2][1], m.m[2][2]));

				Matrix4x4 rot = m;
				rot.m[0][0] /= t.scale.x;
				rot.m[0][1] /= t.scale.x;
				rot.m[0][2] /= t.scale.x;
				rot.m[1][0] /= t.scale.y;
				rot.m[1][1] /= t.scale.y;
				rot.m[1][2] /= t.scale.y;
				rot.m[2][0] /= t.scale.z;
				rot.m[2][1] /= t.scale.z;
				rot.m[2][2] /= t.scale.z;
				t.rotate.y = asin(-rot.m[0][2]);
				t.rotate.x = atan2(rot.m[1][2], rot.m[2][2]);
				t.rotate.z = atan2(rot.m[0][1], rot.m[0][0]);
				t.translate.x = m.m[3][0];
				t.translate.y = m.m[3][1];
				t.translate.z = m.m[3][2];

				Vector3 deltaPos = t.translate - beginTransform_.translate;
				Vector3 deltaRot = t.rotate - beginTransform_.rotate;
				Vector3 deltaScale = t.scale - beginTransform_.scale; 
				deltaTransform_ = { deltaScale, deltaRot, deltaPos };

				target->SetTransforms(gizmoCtx_.editingInstance, t);
			}
		} else {
			// 離したとき
			if ((gizmoCtx_.editAllInstances || gizmoCtx_.useRangeSelect) && wasUsing_) {

				if (auto* target = dynamic_cast<InstancedModel*>(gizmoCtx_.target)) {
					auto transforms = target->GetTransforms();

					for (int i = 0; i < transforms.size(); i++) {

						if (i == gizmoCtx_.editingInstance) continue;

						if (gizmoCtx_.editAllInstances) {
							transforms[i].translate += deltaTransform_.translate;
							transforms[i].rotate += deltaTransform_.rotate;
							transforms[i].scale += deltaTransform_.scale;
							target->SetTransforms(i, transforms[i]);

						} else if(gizmoCtx_.useRangeSelect) {
							if (gizmoCtx_.minRange <= i && gizmoCtx_.maxRange >= i) {
								transforms[i].translate += deltaTransform_.translate;
								transforms[i].rotate += deltaTransform_.rotate;
								transforms[i].scale += deltaTransform_.scale;
								target->SetTransforms(i, transforms[i]);
							}
						}
					}

				}
			}
		}

	}

	wasUsing_ = ImGuizmo::IsUsing();

	ImGui::End();
#endif
}
