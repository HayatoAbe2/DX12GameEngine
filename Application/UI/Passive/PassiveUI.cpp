#include "PassiveUI.h"
#include "Objects/Character/Player/Player.h"
#include "Objects/Item/Passive/Passive.h"

PassiveUI::PassiveUI(Player* player) {
	player_ = player;
}

void PassiveUI::Draw() {
	auto& ctx = GameContext::GetInstance();

	// 所持中アイテム
	auto equip = player_->GetPassives();
	for (int i = 0; i < int(equip.size()); ++i) {
		// 位置
		Vector2 itemPos = equip[i]->GetSprite()->GetPosition();
		Vector2 itemSize = equip[i]->GetSprite()->GetSize();
		AABB2D itemArea = { itemPos - itemSize / 2.0f, itemPos + itemSize / 2.0f };

		if (CheckCollision(itemArea, ctx.Input().mouse.GetPosition())) {
			equip[i]->GetExplain()->SetPosition(itemPos + kExplainOffset);
			equip[i]->GetBackground()->SetPosition(itemPos + kExplainOffset);
			equip[i]->DrawExplain();
		}
	}
}
