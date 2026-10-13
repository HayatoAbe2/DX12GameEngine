#include "WorldPassive.h"

WorldPassive::WorldPassive(std::unique_ptr<Passive> passive, Vector3 pos, Rarity rarity, bool isForSale) :
	WorldItem(pos, rarity, isForSale),
	passive_(std::move(passive)) {

	if (isForSale_) {
		auto& ctx = GameContext::GetInstance();
		price_ = int(price_ * ctx.RandomFloat(valueRangeMin_, valueRangeMax_));
	}
}

std::unique_ptr<Passive> WorldPassive::GetPassiveItem() {
	return std::move(passive_);
}

void WorldPassive::Update() {
}

void WorldPassive::Draw() {
	Sprite* sprite = passive_->GetSprite();
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	auto camera = render.GetCamera();

	// スクリーン変換
	Vector4 clipPos = TransformVector(Vector4(position_.x, 0.5f, position_.y, 1.0f), (camera->viewMatrix_ * camera->projectionMatrix_));

	Vector3 ndc;
	ndc.x = clipPos.x / clipPos.w;
	ndc.y = clipPos.y / clipPos.w;
	ndc.z = clipPos.z / clipPos.w;

	Vector2 window = ctx.GetRenderWindowSize();
	Vector2 pos;
	pos.x = (ndc.x + 1.0f) * 0.5f * window.x;
	pos.y = (1.0f - ndc.y) * 0.5f * window.y;

	// アイコン
	sprite->SetPosition(pos - sprite->GetSize() / 2.0f);
	render.DrawSprite(sprite);

	WorldItem::Draw();
}

void WorldPassive::OnPickup(Player* player) {
	if (isForSale_) {
		Wallet& wallet = player->GetWallet();
		if (!wallet.Pay(price_)) {
			// 所持金不足
			return;
		}
	}

	player->AddPassive(std::move(passive_));

	// ワールド上からはなくなる
	isDead_ = true;
}
