#include "PassiveUI.h"
#include "Objects/Character/Player/Player.h"
#include "Objects/Item/Passive/Passive.h"
#include "Controller.h"

PassiveUI::PassiveUI(Player* player) {
	player_ = player;
}

void PassiveUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto input = ctx.Input();

	// 所持中アイテム
	auto equip = player_->GetPassives();
	if (equip.empty()) return;

	if (input.gamepad.IsConnected()) {
		padSelectTimer_.Update();

		// 他の入力をしたら非表示
		if ((input.gamepad.IsAnyTrigger() || Length(input.gamepad.GetLeftStick()) >= kStickDeadzone) &&
			padSelectTimer_.IsFinished()) {
			isSelectShow_ = false;
		}

		// 十字キーで表示
		if (input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_LEFT) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_RIGHT)) {
			isSelectShow_ = true;
			padSelectTimer_.Start(kPadSelectTime);

			if (input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_LEFT)) {
				select_--;
			} else if (input.gamepad.IsTrigger(XINPUT_GAMEPAD_DPAD_RIGHT)) {
				select_++;
			}
		}

		select_ = std::clamp(select_, 0, int(equip.size()) - 1);
		if (isSelectShow_ && equip[select_]->GetExplain()) {
			Vector2 pos = equip[select_]->GetSprite()->GetPosition();
			equip[select_]->GetExplain()->SetPosition(pos + kExplainOffset);
			equip[select_]->GetBackground()->SetPosition(pos + kExplainOffset);
			equip[select_]->DrawExplain();
		}

	} else {
		for (int i = 0; i < int(equip.size()); ++i) {
			// 位置
			Vector2 itemPos = equip[i]->GetSprite()->GetPosition();
			Vector2 itemSize = equip[i]->GetSprite()->GetSize();
			AABB2D itemArea = { itemPos - itemSize / 2.0f, itemPos + itemSize / 2.0f };

			if (CheckCollision(itemArea, ctx.Input().mouse.GetPosition())) {
				// マウスが重なっている間表示
				if (equip[i]->GetExplain()) {
					equip[i]->GetExplain()->SetPosition(itemPos + kExplainOffset);
					equip[i]->GetBackground()->SetPosition(itemPos + kExplainOffset);
					equip[i]->DrawExplain();
				}
			}
		}
	}

}
