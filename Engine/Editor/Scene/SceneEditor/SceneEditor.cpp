#include "SceneEditor.h"

void SceneEditor::Update() {
}

void SceneEditor::Draw() {
#ifdef USE_IMGUI
    if (scene_) {
        ImGui::Begin("SceneEditor");
        for (auto* object : scene_->GetObjects()) {
            ImGui::PushID(object);

            if (ImGui::CollapsingHeader(object->GetName().c_str())) {
                if (ImGui::CollapsingHeader("Transform")) {
                    // モデル
                    if (auto* model = dynamic_cast<Model*>(object)) {
                        Transform transform = model->GetTransform();
                        ImGui::DragFloat3("Scale", &transform.scale.x);
                        ImGui::DragFloat3("Rotate", &transform.rotate.x);
                        ImGui::DragFloat3("Translate", &transform.translate.x);
                        model->SetTransform(transform);
                    }

                    // スプライト
                    if (auto* sprite = dynamic_cast<Sprite*>(object)) {
                        Vector2 position = sprite->GetPosition();
                        Vector2 size = sprite->GetSize();
                        float rotate = sprite->GetRotation();
                        Vector4 color = sprite->GetColor();
                        ImGui::DragFloat2("Position", &position.x);
                        ImGui::DragFloat2("Size", &size.x);
                        ImGui::DragFloat("Rotate", &rotate);
                        ImGui::DragFloat4("Color", &color.x);
                        sprite->SetPosition(position);
                        sprite->SetSize(size);
                        sprite->SetRotation(rotate);
                        sprite->SetColor(color);
                    }
                }
            }

            ImGui::PopID();
        }
        ImGui::End();
    }
#endif
}
