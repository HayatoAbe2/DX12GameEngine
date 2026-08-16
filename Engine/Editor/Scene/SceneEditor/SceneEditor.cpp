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

#ifdef USE_IMGUI
	// クリックで選択
	if (!ImGuizmo::IsUsing() &&
		GameContext::GetInstance().Input().mouse.IsRelease(MouseButton::Left) &&
		scene_->IsEditMode()) {
		ClickSelect();
	} else if (scene_ && !scene_->IsEditMode()) {
		selected_ = nullptr;
	}

	if (undoRequest_) {
		auto snapshot = undoStack_.back();
		undoStack_.pop_back();
		DeserializeScene(snapshot);
		undoRequest_ = false;
	}
	if (redoRequest_) {
		auto snapshot = redoStack_.back();
		redoStack_.pop_back();
		DeserializeScene(snapshot);
		redoRequest_ = false;
	}
#endif
}

void SceneEditor::Draw(Camera* camera) {
#ifdef USE_IMGUI
	if (scene_) {
		// 一覧
		ImGui::Begin("Hierarchy");
		for (auto* object : scene_->GetObjects()) {
			bool selected = (selected_ == object);

			if (ImGui::Selectable((object->name + "(" + object->tag + ")" + "##" + std::to_string(object->GetID())).c_str(), selected)) {
				selected_ = object;
				gizmoCtx_.editingInstance = 0;
			}
		}
		ImGui::End();

		// 詳細
		ImGui::Begin("Inspector");
		if (selected_) {
			DrawInspector(selected_);

			ImGui::Separator();
			// 追加
			if (auto* model = dynamic_cast<Model*>(selected_)) {
			}
			ImGui::Separator();

			// 削除
			if (ImGui::Button("削除")) {
				PushUndo();
				scene_->RemoveObject(selected_);
				selected_ = nullptr;
			}
		}
		ImGui::End();

		// 編集中切り替え
		ImGui::Begin("Mode");
		bool editMode = scene_->IsEditMode();
		if (ImGui::Checkbox("編集モード", &editMode)) {
			scene_->SetEditMode(editMode);
		}
		ImGui::End();

		ImGui::Begin("セーブ/ロード");

		ImGui::TextUnformatted("resources/");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("##filename", &currentFileName_);
		ImGui::SameLine();
		ImGui::TextUnformatted(".json");

		if (ImGui::Button("セーブ")) {
			Save();
		}
		if (ImGui::Button("ロード")) {
			selected_ = nullptr;

			scene_->Clear();
			if (std::filesystem::exists("Resources/Debug/SceneEditor/" + currentFileName_ + ".json")) {
				Load("Resources/Debug/SceneEditor/" + currentFileName_ + ".json");
			}
		}
		if (ImGui::Button("追加ロード")) {
			selected_ = nullptr;

			if (std::filesystem::exists("Resources/Debug/SceneEditor/" + currentFileName_ + ".json")) {
				Load("Resources/Debug/SceneEditor/" + currentFileName_ + ".json");
			} else {
				scene_->Clear();
			}
		}
		ImGui::End();

		// アセットブラウザ
		ImGui::Begin("Asset Brouser");
		const char* items[]
		{
			"WhiteCube",
			"Wall",
			"Floor"
		};
		ImGui::Combo(
			"Model",
			&selectedAssetIndex_,
			items,
			IM_ARRAYSIZE(items));

		ImGui::InputInt("インスタンス数", &addInstanceNum_);

		if (ImGui::Button("追加")) {
			auto& asset = GameContext::GetInstance().Asset();
			PushUndo();
			scene_->AddObject(asset.LoadInstancedModel(selectedAssetDir_[selectedAssetIndex_], selectedAssetPath_[selectedAssetIndex_], addInstanceNum_));
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

			float view[16];
			float proj[16];
			ToFloat16(view, (camera->viewMatrix_));
			ToFloat16(proj, (camera->projectionMatrix_));
			if (auto* model = dynamic_cast<Model*>(selected_)) {
				float modelMat[16];
				ToFloat16(modelMat, (MakeAffineMatrix(model->GetTransform())));


				if (ImGui::RadioButton("Move", gizmoMode_ == 0)) gizmoMode_ = 0;
				ImGui::SameLine();
				if (ImGui::RadioButton("Rotate", gizmoMode_ == 1)) gizmoMode_ = 1;
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale", gizmoMode_ == 2)) gizmoMode_ = 2;

				// スナップ設定
				ImGui::Checkbox("Snap", &useSnap_);
				switch (gizmoMode_) {
				case 0:
					ImGui::DragFloat("Move Snap", &moveSnap_, 0.1f, 0.01f, 100.0f);
					break;

				case 1:
					ImGui::DragFloat("Rotate Snap", &rotateSnap_, 1.0f, 1.0f, 180.0f);
					break;

				case 2:
					ImGui::DragFloat("Scale Snap", &scaleSnap_, 0.01f, 0.01f, 10.0f);
					break;
				}

				ImGuizmo::OPERATION op;
				switch (gizmoMode_) {
				case 0:
					op = ImGuizmo::TRANSLATE;
					gizmoCtx_.snap[0] = moveSnap_;
					gizmoCtx_.snap[1] = moveSnap_;
					gizmoCtx_.snap[2] = moveSnap_;
					break;
				case 1:
					op = ImGuizmo::ROTATE;
					gizmoCtx_.snap[0] = rotateSnap_;
					break;
				case 2:
					op = ImGuizmo::SCALE;
					gizmoCtx_.snap[0] = scaleSnap_;
					break;
				}
				gizmoCtx_.isActive = selected_ != nullptr;
				gizmoCtx_.target = selected_;
				gizmoCtx_.op = op;
				for (int i = 0; i < 16; ++i) {
					gizmoCtx_.view[i] = view[i];
					gizmoCtx_.proj[i] = proj[i];
					gizmoCtx_.modelMatrix[i] = modelMat[i];
				};
				gizmoCtx_.useSnap = useSnap_;

			} else if (auto* model = dynamic_cast<InstancedModel*>(selected_)) {
				auto transforms = model->GetTransforms();

				// アクセス範囲修正
				if (int(transforms.size()) <= editingInstance_) editingInstance_ = int(transforms.size()) - 1;

				float modelMat[16];
				ToFloat16(modelMat, MakeAffineMatrix(transforms[editingInstance_]));

				if (ImGui::RadioButton("Move", gizmoMode_ == 0)) gizmoMode_ = 0;
				ImGui::SameLine();
				if (ImGui::RadioButton("Rotate", gizmoMode_ == 1)) gizmoMode_ = 1;
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale", gizmoMode_ == 2)) gizmoMode_ = 2;

				// スナップ設定
				ImGui::Checkbox("Snap", &useSnap_);
				switch (gizmoMode_) {
				case 0:
					ImGui::DragFloat("Move Snap", &moveSnap_, 0.1f, 0.01f, 100.0f);
					break;

				case 1:
					ImGui::DragFloat("Rotate Snap", &rotateSnap_, 1.0f, 1.0f, 180.0f);
					break;

				case 2:
					ImGui::DragFloat("Scale Snap", &scaleSnap_, 0.01f, 0.01f, 10.0f);
					break;
				}
				ImGui::Checkbox("UseRangeSelect", &gizmoCtx_.useRangeSelect);
				if (gizmoCtx_.useRangeSelect) {
					ImGui::DragInt("Min", &gizmoCtx_.minRange, 1, 0, int(transforms.size()));
					ImGui::DragInt("Max", &gizmoCtx_.maxRange, 1, 0, int(transforms.size()));
				}
				ImGui::Checkbox("EditAllInstances", &gizmoCtx_.editAllInstances);

				ImGuizmo::OPERATION op;
				switch (gizmoMode_) {
				case 0:
					op = ImGuizmo::TRANSLATE;
					gizmoCtx_.snap[0] = moveSnap_;
					gizmoCtx_.snap[1] = moveSnap_;
					gizmoCtx_.snap[2] = moveSnap_;
					break;
				case 1:
					op = ImGuizmo::ROTATE;
					gizmoCtx_.snap[0] = rotateSnap_;
					break;
				case 2:
					op = ImGuizmo::SCALE;
					gizmoCtx_.snap[0] = scaleSnap_;
					break;
				}
				gizmoCtx_.isActive = selected_ != nullptr;
				gizmoCtx_.target = selected_;
				gizmoCtx_.op = op;
				for (int i = 0; i < 16; ++i) {
					gizmoCtx_.view[i] = view[i];
					gizmoCtx_.proj[i] = proj[i];
					gizmoCtx_.modelMatrix[i] = modelMat[i];
				};
				gizmoCtx_.useSnap = useSnap_;
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
	auto& input = GameContext::GetInstance().Input();

	ImGui::InputText("tag", &object->tag);

	// トランスフォーム等
	if (ImGui::CollapsingHeader("Transform") && !ImGuizmo::IsUsing()) {
		// モデル
		if (auto* model = dynamic_cast<Model*>(object)) {
			Transform transform = model->GetTransform();
			ImGui::DragFloat3("Scale", &transform.scale.x, gizmoCtx_.snap[2]);
			ImGui::DragFloat3("Rotate", &transform.rotate.x, gizmoCtx_.snap[1]);
			ImGui::DragFloat3("Translate", &transform.translate.x, gizmoCtx_.snap[0]);
			model->SetTransform(transform);
		}

		// インスタンシング描画モデル
		if (auto* model = dynamic_cast<InstancedModel*>(object)) {
			auto transforms = model->GetTransforms();

			Vector3 addTranslate = {};
			if (input.keyboard.IsTrigger(DIK_LEFT)) {
				addTranslate.x = -moveSnap_;
			}
			if (input.keyboard.IsTrigger(DIK_RIGHT)) {
				addTranslate.x = moveSnap_;
			}
			if (input.keyboard.IsTrigger(DIK_UP)) {
				addTranslate.z = moveSnap_;
			}
			if (input.keyboard.IsTrigger(DIK_DOWN)) {
				addTranslate.z = -moveSnap_;
			}

			if (input.keyboard.IsTrigger(DIK_LEFT) || input.keyboard.IsTrigger(DIK_RIGHT) ||
				input.keyboard.IsTrigger(DIK_UP) || input.keyboard.IsTrigger(DIK_DOWN)) {

				// 次インスタンス選択
				if (transforms.size() > editingInstance_ + 1 && input.keyboard.IsPress(DIK_LSHIFT)) {
					editingInstance_++;

					// 移動
					if (gizmoCtx_.useRangeSelect) {
						int r = gizmoCtx_.maxRange + 1 - gizmoCtx_.minRange;
						for (int i = gizmoCtx_.minRange; i <= gizmoCtx_.maxRange; i++) {
							// transformコピー
							if (i + r < transforms.size()) {
								model->SetTransforms(i + r, transforms[i]);
							}
						}

						if (gizmoCtx_.minRange + r < transforms.size()) {
							gizmoCtx_.minRange += r;
							gizmoCtx_.maxRange += r;
						}
					} else {
						// transformコピー
						transforms[editingInstance_] = transforms[editingInstance_ - 1];
					}
				}

				transforms[editingInstance_].translate += addTranslate;
			}

			int count = model->GetNumInstance();
			ImGui::DragInt("Instance", &editingInstance_, 1, 0, count - 1);

			if (int(transforms.size()) <= editingInstance_) editingInstance_ = int(transforms.size()) - 1;
			gizmoCtx_.editingInstance = editingInstance_;

			Transform t = transforms[editingInstance_];

			ImGui::DragFloat3("Scale", &t.scale.x, gizmoCtx_.snap[2]);
			ImGui::DragFloat3("Rotate", &t.rotate.x, gizmoCtx_.snap[0]);
			ImGui::DragFloat3("Translate", &t.translate.x, gizmoCtx_.snap[0]);

			model->SetTransforms(editingInstance_, t);
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
#ifdef USE_IMGUI
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();

	for (auto* object : scene_->GetObjects()) {
		if (auto* model = dynamic_cast<Model*>(object)) {
			nlohmann::json object;
			object["type"] = "Model";
			object["name"] = model->name;
			object["tag"] = model->tag;
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
		} else if (auto* model = dynamic_cast<InstancedModel*>(object)) {

			nlohmann::json object;
			object["type"] = "InstancedModel";
			object["name"] = model->name;
			object["tag"] = model->tag;
			object["path"] = model->GetDirectoryPath();

			object["transforms"] = nlohmann::json::array();

			for (const auto& t : model->GetTransforms()) {

				object["transforms"].push_back({
					{"scale", {
						t.scale.x,
						t.scale.y,
						t.scale.z
					}},
					{"rotate", {
						t.rotate.x,
						t.rotate.y,
						t.rotate.z
					}},
					{"translate", {
						t.translate.x,
						t.translate.y,
						t.translate.z
					}}
					});
			}

			root["objects"].push_back(object);
		}
	}

	std::ofstream file("Resources/Debug/SceneEditor/" + currentFileName_ + ".json");
	file << root.dump(4);
#endif
}

void SceneEditor::Load(const std::string& path, Vector3 offset) {
	auto& asset = GameContext::GetInstance().Asset();

	std::ifstream file(path);
	nlohmann::json root;
	file >> root;

	for (auto& objectJson : root["objects"]) {
		std::string type = objectJson["type"];

		// モデル
		if (type == "Model") {
			auto model = asset.LoadModel(objectJson["path"], objectJson["name"]);
			model->tag = objectJson["tag"];

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
			translate.x = objectJson["translate"][0] + offset.x;
			translate.y = objectJson["translate"][1] + offset.y;
			translate.z = objectJson["translate"][2] + offset.z;
			model->SetTransform({ scale, rotate, translate });

			scene_->AddObject(std::move(model));
		} else if (type == "InstancedModel") {
			std::unique_ptr<InstancedModel> model;

			model = asset.LoadInstancedModel(objectJson["path"], objectJson["name"], int(objectJson["transforms"].size()));
			model->tag = objectJson["tag"];

			std::vector<Transform> transforms;
			for (auto& json : objectJson["transforms"]) {

				Transform t;

				t.scale = {
					json["scale"][0],
					json["scale"][1],
					json["scale"][2]
				};

				t.rotate = {
					json["rotate"][0],
					json["rotate"][1],
					json["rotate"][2]
				};

				t.translate = {
					json["translate"][0],
					json["translate"][1],
					json["translate"][2]
				};
				t.translate += offset;

				transforms.push_back(t);
			}

			model->SetTransforms(transforms);
			scene_->AddObject(std::move(model));
		}
	}
}

std::string SceneEditor::SerializeScene() {
#ifdef USE_IMGUI
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();

	for (auto* object : scene_->GetObjects()) {
		if (auto* model = dynamic_cast<Model*>(object)) {
			nlohmann::json object;
			object["type"] = "Model";
			object["name"] = model->name;
			object["tag"] = model->tag;
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
		} else if (auto* model = dynamic_cast<InstancedModel*>(object)) {
			nlohmann::json object;
			object["type"] = "InstancedModel";
			object["name"] = model->name;
			object["tag"] = model->tag;
			object["path"] = model->GetDirectoryPath();

			object["transforms"] = nlohmann::json::array();

			for (const auto& t : model->GetTransforms()) {
				object["transforms"].push_back({
					{"scale", {
						t.scale.x,
						t.scale.y,
						t.scale.z
					}},
					{"rotate", {
						t.rotate.x,
						t.rotate.y,
						t.rotate.z
					}},
					{"translate", {
						t.translate.x,
						t.translate.y,
						t.translate.z
					}}
					});
			}

			root["objects"].push_back(object);
		}
	}

	return root.dump(4);
#endif
}

void SceneEditor::DeserializeScene(const std::string& jsonText) {
#ifdef USE_IMGUI
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
			model->tag = objectJson["tag"];

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
		} else if (type == "InstancedModel") {

			auto& asset = GameContext::GetInstance().Asset();
			int count = static_cast<int>(objectJson["transforms"].size());
			auto model = asset.LoadInstancedModel(
				objectJson["path"],
				objectJson["name"], count);
			model->tag = objectJson["tag"];

			for (int i = 0; i < count; i++) {
				Transform t;

				auto& json = objectJson["transforms"][i];

				t.scale = {
					json["scale"][0],
					json["scale"][1],
					json["scale"][2]
				};

				t.rotate = {
					json["rotate"][0],
					json["rotate"][1],
					json["rotate"][2]
				};

				t.translate = {
					json["translate"][0],
					json["translate"][1],
					json["translate"][2]
				};

				model->SetTransforms(i, t);
			}

			scene_->AddObject(std::move(model));
		}
	}
#endif
}

void SceneEditor::PushUndo() {
#ifdef USE_IMGUI
	undoStack_.push_back({
		SerializeScene()
		});

	redoStack_.clear();
#endif
}

void SceneEditor::Undo() {
#ifdef USE_IMGUI
	if (undoStack_.empty()) {
		return;
	}

	redoStack_.push_back({
		SerializeScene()
		});

	undoRequest_ = true;
#endif
}

void SceneEditor::Redo() {
#ifdef USE_IMGUI
	if (redoStack_.empty()) {
		return;
	}

	undoStack_.push_back({
		SerializeScene()
		});

	redoRequest_ = true;
#endif
}

void SceneEditor::ClickSelect() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	Camera* camera = ctx.Render().GetCamera();

	Vector2 windowSize = ctx.GetWindowSize();
	Vector2 mousePos = input.mouse.GetPosition();

	float ndcX = (mousePos.x / windowSize.x) * 2.0f - 1.0f;
	float ndcY = 1.0f - (mousePos.y / windowSize.y) * 2.0f;

	Vector4 nearPoint = { ndcX, ndcY, 0, 1 };
	Vector4 farPoint = { ndcX, ndcY, 1, 1 };

	Matrix4x4 inverseVP = Inverse(camera->viewMatrix_ * camera->projectionMatrix_);

	nearPoint = TransformVector(nearPoint, inverseVP);
	farPoint = TransformVector(farPoint, inverseVP);
	nearPoint /= nearPoint.w;
	farPoint /= farPoint.w;

	// マウス位置に向けたレイ
	Ray3D ray;
	ray.origin = { nearPoint.x, nearPoint.y, nearPoint.z };
	ray.direction = Normalize(Vector3{ {farPoint.x - nearPoint.x},{farPoint.y - nearPoint.y},{farPoint.z - nearPoint.z} }); // 方向ベクトル

	float nearest = FLT_MAX;

	auto objects = scene_->GetObjects();
	for (auto& object : objects) {
		if (auto* model = dynamic_cast<Model*>(object)) {
			Vector3 pos = model->GetTransform().translate;
			Vector3 size = model->GetTransform().scale;
			AABB3D aabb = { pos - size / 2.0f, pos + size / 2.0f };
			if (CheckCollision(aabb, ray)) {
				float  distance = Length(pos - ray.origin);
				if (nearest > distance) {
					nearest = distance;

					selected_ = model;
				}
			}

		} else if (auto* model = dynamic_cast<InstancedModel*>(object)) {
			std::vector<Transform> transforms;

			// 全インスタンスのトランスフォーム追加
			for (auto& transform : model->GetTransforms()) {
				transforms.push_back(transform);
			}

			for (int i = 0; i < transforms.size(); ++i) {
				Vector3 pos = transforms[i].translate;
				Vector3 size = transforms[i].scale;
				AABB3D aabb = { pos - size / 2.0f, pos + size / 2.0f };
				if (CheckCollision(aabb, ray)) {
					float  distance = Length(transforms[i].translate - ray.origin);
					if (nearest > distance) {
						nearest = distance;

						selected_ = model;
						editingInstance_ = i;
					}
				}
			}
		}
	}
}