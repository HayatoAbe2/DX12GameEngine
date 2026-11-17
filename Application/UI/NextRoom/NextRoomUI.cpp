#include "NextRoomUI.h"
#include "Map/FloorManager/FloorManager.h"

NextRoomUI::NextRoomUI(FloorManager* floorManager) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 画像ロード
	icons_.resize(3);
	for (int i = 0; i < 3; ++i) {
		icons_[i] = asset.LoadSprite("Resources/Images/UI/Icons/dice_question.png");
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
		Vector4 color = { 1,1,1,1 };

		// 次の部屋タイプ
		switch (connectors[i].connectedRoom->type) {
		case RoomType::Elite:
			color = { 1,0,0,1 };
			[[fallthrough]];
		case RoomType::Combat:
			name = "sword";
			break;
		case RoomType::Boss:
			name = "skull";
			color = { 0.5f,0,1,1 };
			break;
		case RoomType::Shop:
			name = "pouch";
			break;
		case RoomType::Goal:
			name = "flag_triangle";
			break;
		}
		// アイコン設定
		icons_[i] = asset.LoadSprite(std::string("Resources/Images/UI/Icons/") + name + std::string(".png"));
		icons_[i]->SetSize({ iconSize_, iconSize_ });
		icons_[i]->SetPivot({ 0.5f,0.5f });
		icons_[i]->SetColor(color);
	}

	auto& render = ctx.Render();
	auto camera = render.GetCamera();
	roomState_ = floorManager_->GetCurrentRoomState();
	if (roomState_ != RoomState::Cleared) {
		// アイコンはその位置に表示
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
			icons_[i]->SetPosition(pos);
		}
	} else {
		// アイコンを画面外にいても表示
		Vector2 window = ctx.GetRenderWindowSize();

		// 画面端からアイコンを離す余白
		const float margin = 64.0f;

		rewardIcons_.resize(int(connectors.size()));
		for (int i = 0; i < int(connectors.size()); ++i) {
			// 出入口の位置
			Vector2 position =
				(connectors[i].collider.min + connectors[i].collider.max) / 2.0f;

			// スクリーン変換
			Vector4 clipPos = TransformVector(Vector4(position.x, 0.5f, position.y, 1.0f), camera->viewMatrix_ * camera->projectionMatrix_);

			// NDC座標
			Vector2 ndc;
			ndc.x = clipPos.x / clipPos.w;
			ndc.y = clipPos.y / clipPos.w;

			// NDC座標からスクリーン座標へ変換
			Vector2 pos;
			pos.x = (ndc.x + 1.0f) * 0.5f * window.x;
			pos.y = (1.0f - ndc.y) * 0.5f * window.y;

			// 画面中央からの方向ベクトル
			Vector2 center = window * 0.5f;
			Vector2 direction = pos - center;

			// 画面内に収めるための最大倍率を計算
			float scaleX = (window.x * 0.5f - margin) / max(std::abs(direction.x), 0.0001f);
			float scaleY = (window.y * 0.5f - margin) / max(std::abs(direction.y), 0.0001f);
			float scale = (std::min)({ scaleX, scaleY, 1.0f });

			// 方向を維持したまま画面内に配置
			pos = center + direction * scale;

			// アイコン
			icons_[i]->SetPosition(pos);

			// 報酬内容
			const RoomPlan& plan = connectors[i].connectedRoom->plan;

			if (plan.planned && plan.reward.type != RewardType::None) {
				std::string rewardName; // 画像名
				switch (plan.reward.type) {
				case RewardType::Money:   rewardName = "coin";    break;
				case RewardType::Weapon:  rewardName = "weapon";  break;
				case RewardType::Passive: rewardName = "item"; break;
				}
				rewardIcons_[i] = asset.LoadSprite(std::string("Resources/Images/UI/Icons/") + rewardName + ".png");
				rewardIcons_[i]->SetSize({ rewardIconSize_, rewardIconSize_ });
				rewardIcons_[i]->SetPivot({ 0.5f, 0.5f });
				rewardIcons_[i]->SetPosition({ pos + rewardOffset_ });

				// レアリティ色
				Vector4 color = { 1, 1, 1, 1 };
				if (plan.reward.type == RewardType::Weapon) {
					switch (plan.reward.rarity) {
					case Rarity::Rare:      color = { 0.4f, 0.6f, 1.0f, 1 }; break;
					case Rarity::Epic:      color = { 0.7f, 0.4f, 1.0f, 1 }; break;
					case Rarity::Legendary: color = { 1.0f, 0.8f, 0.2f, 1 }; break;
					default: break;
					}
				}
				rewardIcons_[i]->SetColor(color);
			}
		}
	}
}

void NextRoomUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	auto connectors = floorManager_->GetConnector();
	for (int i = 0; i < int(connectors.size()); ++i) {
		render.DrawSprite(icons_[i].get());
		if (roomState_ == RoomState::Cleared && i < rewardIcons_.size() && rewardIcons_[i]) {
			render.DrawSprite(rewardIcons_[i].get());
		}
	}
}
