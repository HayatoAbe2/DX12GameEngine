#include "PassiveUI.h"
#include "Objects/Character/Player/Player.h"
#include "Objects/Item/Passive/Passive.h"

PassiveUI::PassiveUI(Player* player) {
	player_ = player;
}

void PassiveUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto input = ctx.Input();

	// 所持中アイテム
	auto equip = player_->GetPassives();

	for (int i = 0; i < int(equip.size()); ++i) {
		// 位置
		Vector2 itemPos = equip[i]->GetSprite()->GetPosition();
		Vector2 itemSize = equip[i]->GetSprite()->GetSize();
		AABB2D itemArea = { itemPos - itemSize / 2.0f, itemPos + itemSize / 2.0f }; 

		if (input.gamepad.IsConnected()) {
			padSelectTimer_.Update();

			// 他の入力をしたら非表示
			if (input.gamepad.IsAnyTrigger() && padSelectTimer_.IsFinished()) {
				isSelectShow_ = false;
			}

			//// 十字キーで表示
			//if (input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_LEFT) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_LEFT)) {
			//	isSelectShow_ = true;
			//	padSelectTimer_.Start(kPadSelectTime);
			//}

			//select_ = std::clamp(select_, 0, int(equip.size()));
			//if (equip[select_]->GetExplain()) {
			//	equip[select_]->GetExplain()->SetPosition(itemPos + kExplainOffset);
			//	equip[select_]->GetBackground()->SetPosition(itemPos + kExplainOffset);
			//	equip[select_]->DrawExplain();
			//}
		} else if (CheckCollision(itemArea, ctx.Input().mouse.GetPosition())) {
			if (equip[i]->GetExplain()) {
				equip[i]->GetExplain()->SetPosition(itemPos + kExplainOffset);
				equip[i]->GetBackground()->SetPosition(itemPos + kExplainOffset);
				equip[i]->DrawExplain();
			}
		}
	}
}
