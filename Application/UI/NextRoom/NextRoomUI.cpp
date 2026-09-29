#include "NextRoomUI.h"
#include "Map/FloorManager/FloorManager.h"

NextRoomUI::NextRoomUI(FloorManager* floorManager) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	icons_.resize(3);
	for (int i = 0; i < 3; ++i) {
		icons_[i] = asset.LoadSprite("Resources/UI/Icons/dice_question.png");
		icons_[i]->SetSize({ iconSize_, iconSize_ });
		icons_[i]->SetPivot({ 0.5f,0.5f });
	}

	floorManager_ = floorManager;
}

void NextRoomUI::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	auto connectors = floorManager_->GetConnector();
	for (int i = 0; i < int(connectors.size()); ++i) {
		// ファイル名
		std::string name = "dice_question";

		// 次の部屋タイプ
		switch (connectors[i].connectedRoom->type) {
		case RoomType::Combat:
			name = "sword";
			break;
		case RoomType::Shop:
			name = "pouch";
			break;
		case RoomType::Goal:
			name = "flag_triangle";
			break;
		}
		// アイコン設定
		icons_[i] = asset.LoadSprite(std::string("Resources/UI/Icons/") + name + std::string(".png"));
		icons_[i]->SetSize({ iconSize_, iconSize_ });
		icons_[i]->SetPivot({ 0.5f,0.5f });
	}
}

void NextRoomUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	auto camera = render.GetCamera();

	auto connectors = floorManager_->GetConnector();
	for (int i = 0; i < int(connectors.size()); ++i) {
		// 出入口の位置
		Vector2 position = (connectors[i].collider.min + connectors[i].collider.max) / 2.0f;

		// スクリーン変換
		Vector4 clipPos = TransformVector(Vector4(position.x, 0.5f, position.y, 1.0f), (camera->viewMatrix_ * camera->projectionMatrix_));

		Vector3 ndc;
		ndc.x = clipPos.x / clipPos.w;
		ndc.y = clipPos.y / clipPos.w;
		ndc.z = clipPos.z / clipPos.w;

		Vector2 window = ctx.GetRenderWindowSize();
		Vector2 pos;
		pos.x = (ndc.x + 1.0f) * 0.5f * window.x;
		pos.y = (1.0f - ndc.y) * 0.5f * window.y;

		// アイコン
		icons_[i]->SetPosition(pos - icons_[i]->GetSize() / 2.0f);
		render.DrawSprite(icons_[i].get());
	}
}
