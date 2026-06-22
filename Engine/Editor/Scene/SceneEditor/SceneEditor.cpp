#include "SceneEditor.h"

void ToFloat16(float out[16], const Matrix4x4& m) {
	out[0] = m.m[0][0];
	out[1] = m.m[0][1];
	out[2] = m.m[0][2];
	out[3] = m.m[0][3];

	out[4] = m.m[1][0];
	out[5] = m.m[1][1];
	out[6] = m.m[1][2];
	out[7] = m.m[1][3];

	out[8] = m.m[2][0];
	out[9] = m.m[2][1];
	out[10] = m.m[2][2];
	out[11] = m.m[2][3];

	out[12] = m.m[3][0];
	out[13] = m.m[3][1];
	out[14] = m.m[3][2];
	out[15] = m.m[3][3];
}

void SceneEditor::Update() {
	if (scene_) {
		scene_->FlushDelete();
	}
}

void SceneEditor::Draw(Camera* camera) {
#ifdef USE_IMGUI
	if (scene_) {
		// 一覧
		ImGui::Begin("Hierarchy");
		for (auto* object : scene_->GetObjects()) {
			bool selected = (selected_ == object);

			if (ImGui::Selectable((object->name_ + "##" + std::to_string(object->GetID())).c_str(), selected)) {
				selected_ = object;
			}
		}
		ImGui::End();

		// 詳細
		ImGui::Begin("Inspector");
		if (selected_) {
			DrawInspector(selected_);

			ImGui::Separator();
			// 削除
			if (ImGui::Button("Delete")) {
				PushUndo();
				scene_->RemoveObject(selected_);
				selected_ = nullptr;
			}
		}
		ImGui::End();

		ImGui::Begin("Save/Load");
		if (ImGui::Button("Save")) {
			Save();
		}
		if (ImGui::Button("Load")) {
			Load("Resources/Debug/SceneEditor/SceneData.json");
		}
		ImGui::End();

		// アセットブラウザ
		ImGui::Begin("Asset Brouser");

		const char* items[]
		{
			"Human",
			"Floor",
		};
		ImGui::Combo(
			"Model",
			&selectedAssetIndex_,
			items,
			IM_ARRAYSIZE(items));

		if (ImGui::Button("Add")) {
			auto& asset = GameContext::GetInstance().Asset();
			PushUndo();
			scene_->AddObject(asset.LoadModel(selectedAssetDir_[selectedAssetIndex_], selectedAssetPath_[selectedAssetIndex_]));
		}
		ImGui::End();

		// ギズモ
		if (selected_) {
			if (ImGuizmo::IsUsing() && !wasUsing_) {
				PushUndo();
			}
			wasUsing_ = ImGuizmo::IsUsing();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::BeginFrame();
	
			if (auto* model = dynamic_cast<Model*>(selected_)) {
				float view[16];
				float proj[16];
				float modelMat[16];
				ToFloat16(view, (camera->viewMatrix_));
				ToFloat16(proj, (camera->projectionMatrix_));
				ToFloat16(modelMat, (MakeAffineMatrix(model->GetTransform())));

				if (ImGui::RadioButton("Move", gizmoMode_ == 0)) gizmoMode_ = 0;
				if (ImGui::RadioButton("Rotate", gizmoMode_ == 1)) gizmoMode_ = 1;
				if (ImGui::RadioButton("Scale", gizmoMode_ == 2)) gizmoMode_ = 2;

				ImGuizmo::OPERATION op;
				switch (gizmoMode_) {
				case 0: op = ImGuizmo::TRANSLATE; break;
				case 1: op = ImGuizmo::ROTATE; break;
				case 2: op = ImGuizmo::SCALE; break;
				}
				gizmoCtx_.isActive = selected_ != nullptr;
				gizmoCtx_.target = selected_;
				gizmoCtx_.op = op;
				for (int i = 0; i < 16; ++i) {
					gizmoCtx_.view[i] = view[i];
					gizmoCtx_.proj[i] = proj[i];
					gizmoCtx_.model[i] = modelMat[i];
				};
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
			ImGui::IsKeyPressed(ImGuiKey_Z)) {
			Undo();
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
			ImGui::IsKeyPressed(ImGuiKey_Y)) {
			Redo();
		}
	}
#endif
}

void SceneEditor::DrawInspector(SceneObject* object) {
#ifdef USE_IMGUI
	// トランスフォーム等
	if (ImGui::CollapsingHeader("Transform") && !ImGuizmo::IsUsing()) {
		// モデル
		if (auto* model = dynamic_cast<Model*>(object)) {
			Transform transform = model->GetTransform();
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.5f);
			ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.5f);
			ImGui::DragFloat3("Translate", &transform.translate.x, 0.5f);
			model->SetTransform(transform);
		}

		// スプライト
		if (auto* sprite = dynamic_cast<Sprite*>(object)) {
			Vector2 position = sprite->GetPosition();
			Vector2 size = sprite->GetSize();
			float rotate = sprite->GetRotation();
			Vector4 color = sprite->GetColor();
			ImGui::DragFloat2("Position", &position.x, 0.5f);
			ImGui::DragFloat2("Size", &size.x, 0.5f);
			ImGui::DragFloat("Rotate", &rotate, 0.5f);
			ImGui::DragFloat4("Color", &color.x, 0.5f);
			sprite->SetPosition(position);
			sprite->SetSize(size);
			sprite->SetRotation(rotate);
			sprite->SetColor(color);
		}
	}
#endif
}

void SceneEditor::Save() {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();

	for (auto* object : scene_->GetObjects()) {
		if (auto* model = dynamic_cast<Model*>(object)) {
			nlohmann::json object;
			object["type"] = "Model";
			object["name"] = model->name_;
			object["path"] = model->GetDirectoryPath();

			Transform transform = model->GetTransform();
			object["translate"] =
			{
				transform.translate.x,
				transform.translate.y,
				transform.translate.z
			};
			object["rotate"] =
			{
				transform.rotate.x,
				transform.rotate.y,
				transform.rotate.z
			};
			object["scale"] =
			{
				transform.scale.x,
				transform.scale.y,
				transform.scale.z
			};

			root["objects"].push_back(object);
		}
	}

	std::ofstream file("Resources/Debug/SceneEditor/SceneData.json");
	file << root.dump(4);
}

void SceneEditor::Load(const std::string& path) {
	std::ifstream file(path);
	nlohmann::json root;
	file >> root;

	for (auto& objectJson : root["objects"]) {
		std::string type = objectJson["type"];

		// モデル
		if (type == "Model") {
			auto& asset = GameContext::GetInstance().Asset();
			auto model = asset.LoadModel(objectJson["path"], objectJson["name"]);

			// Transform
			Vector3 scale;
			scale.x = objectJson["scale"][0];
			scale.y = objectJson["scale"][1];
			scale.z = objectJson["scale"][2];
			Vector3 rotate;
			rotate.x = objectJson["rotate"][0];
			rotate.y = objectJson["rotate"][1];
			rotate.z = objectJson["rotate"][2];
			Vector3 translate;
			translate.x = objectJson["translate"][0];
			translate.y = objectJson["translate"][1];
			translate.z = objectJson["translate"][2];
			model->SetTransform({ scale, rotate, translate });

			scene_->AddObject(std::move(model));
		}
	}
}

std::string SceneEditor::SerializeScene() {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();

	for (auto* object : scene_->GetObjects()) {
		if (auto* model = dynamic_cast<Model*>(object)) {
			nlohmann::json object;
			object["type"] = "Model";
			object["name"] = model->name_;
			object["path"] = model->GetDirectoryPath();

			Transform transform = model->GetTransform();
			object["translate"] =
			{
				transform.translate.x,
				transform.translate.y,
				transform.translate.z
			};
			object["rotate"] =
			{
				transform.rotate.x,
				transform.rotate.y,
				transform.rotate.z
			};
			object["scale"] =
			{
				transform.scale.x,
				transform.scale.y,
				transform.scale.z
			};

			root["objects"].push_back(object);
		}
	}

	return root.dump(4);
}

void SceneEditor::DeserializeScene(const std::string& jsonText) {
	selected_ = nullptr;
	gizmoCtx_.isActive = false;
	gizmoCtx_.target = nullptr;

	nlohmann::json root = nlohmann::json::parse(jsonText);
	scene_->Clear();

	for (auto& objectJson : root["objects"]) {
		std::string type = objectJson["type"];

		// モデル
		if (type == "Model") {
			auto& asset = GameContext::GetInstance().Asset();
			auto model = asset.LoadModel(objectJson["path"], objectJson["name"]);

			// Transform
			Vector3 scale;
			scale.x = objectJson["scale"][0];
			scale.y = objectJson["scale"][1];
			scale.z = objectJson["scale"][2];
			Vector3 rotate;
			rotate.x = objectJson["rotate"][0];
			rotate.y = objectJson["rotate"][1];
			rotate.z = objectJson["rotate"][2];
			Vector3 translate;
			translate.x = objectJson["translate"][0];
			translate.y = objectJson["translate"][1];
			translate.z = objectJson["translate"][2];
			model->SetTransform({ scale, rotate, translate });

			scene_->AddObject(std::move(model));
		}
	}
}

void SceneEditor::PushUndo() {
	undoStack_.push_back({
		SerializeScene()
		});

	redoStack_.clear();
}

void SceneEditor::Undo() {
	if (undoStack_.empty()) {
		return;
	}

	redoStack_.push_back({
		SerializeScene()
		});

	auto snapshot = undoStack_.back();
	undoStack_.pop_back();

	DeserializeScene(snapshot);
}

void SceneEditor::Redo() {
	if (redoStack_.empty()) {
		return;
	}

	undoStack_.push_back({
		SerializeScene()
		});

	auto snapshot = redoStack_.back();
	redoStack_.pop_back();

	DeserializeScene(snapshot);
}