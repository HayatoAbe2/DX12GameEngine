#pragma once
#ifdef USE_IMGUI
#include <optional>
#include <vector>
#include "imgui.h"
#include "Data/DataIssue.h"

/// <summary>
/// エディタ共通のImGui部品
/// </summary>
namespace EditorUtil {

	// 検証結果を表示する。エラーがあれば true
	inline bool DrawIssues(const std::vector<DataIssue>& issues) {
		bool hasError = false;
		ImGui::PushTextWrapPos(0.0f);
		for (const auto& issue : issues) {
			if (issue.isError) {
				hasError = true;
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[エラー] %s", issue.message.c_str());
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "[警告] %s", issue.message.c_str());
			}
		}
		ImGui::PopTextWrapPos();
		return hasError;
	}

	// 有効/無効を切り替えられる項目(チェックボックスで出し入れ)
	template <typename T, typename Body>
	void Section(const char* label, std::optional<T>& value, const T& defaultValue, Body body) {
		bool enabled = value.has_value();
		if (ImGui::Checkbox(label, &enabled)) {
			if (enabled) { value = defaultValue; } else { value.reset(); }
		}
		if (value) {
			ImGui::Indent();
			ImGui::PushID(label);
			body(*value);
			ImGui::PopID();
			ImGui::Unindent();
		}
	}

	// 範囲を守る DragFloat
	inline bool DragFloat(const char* label, float* v, float speed, float min, float max, const char* format = "%.2f") {
		return ImGui::DragFloat(label, v, speed, min, max, format, ImGuiSliderFlags_AlwaysClamp);
	}

	inline bool DragInt(const char* label, int* v, float speed, int min, int max) {
		return ImGui::DragInt(label, v, speed, min, max, "%d", ImGuiSliderFlags_AlwaysClamp);
	}
}
#endif
