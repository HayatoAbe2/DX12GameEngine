#include "Sprite.h"
#include "Engine/Asset/Resource/Material/Material.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

void Sprite::UpdateVertex() {
	float left = -pivot_.x;
	float right = 1.0f - pivot_.x;
	float top = -pivot_.y;
	float bottom = 1.0f - pivot_.y;

	vertexData_[0].position = { left,  bottom, 0.0f, 1.0f };
	vertexData_[1].position = { left,  top,    0.0f, 1.0f };
	vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData_[3].position = { right, top,    0.0f, 1.0f };
}

void Sprite::SetTextureRect(float x, float y, float w, float h) {
	Vector2 texWH = material_->GetTexture()->GetSize();

	float left = x / texWH.x;
	float top = y / texWH.y;
	float right = (x + w) / texWH.x;
	float bottom = (y + h) / texWH.y;

	vertexData_[0].texcoord = { left, bottom };
	vertexData_[1].texcoord = { left, top };
	vertexData_[2].texcoord = { right, bottom };
	vertexData_[3].texcoord = { right, top };
}

void Sprite::ImGuiEdit() {
#ifdef USE_IMGUI
	std::string s = "Sprite##" + std::to_string(id_);
	ImGui::Begin(s.c_str());
	ImGui::DragFloat2("Position", &position_.x, 1.0f);
	ImGui::Checkbox("Keep Aspect", &debugKeepAspect_);
	float aspect = size_.x / size_.y;
	if (ImGui::DragFloat("Width", &size_.x, 1.0f)) {
		if (debugKeepAspect_) {
			size_.y = size_.x / aspect;
		}
	}

	if (ImGui::DragFloat("Height", &size_.y, 1.0f)) {
		if (debugKeepAspect_) {
			size_.x = size_.y * aspect;
		}
	}
	ImGui::DragFloat("Rotation", &rotation_, 1.0f);
	ImGui::End();
#endif
}


