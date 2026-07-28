#include "WorldItem.h"
#include "GameCommon.h"

WorldItem::WorldItem(Vector3 pos, Rarity rarity, bool isForSale) {
	position_ = ToXZ(pos);
	rarity_ = rarity;

	if (rarity == Rarity::None) return;

	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();
	auto& asset = ctx.Asset();

	// アイテム強調ライトの追加
	if (rarity_ > Rarity::None) {
		lightIndex_ = light.AddPointLight();
		if (lightIndex_ >= 0) {
			auto& pointLight = light.GetPointLight(lightIndex_);
			pointLight.position = pos;
			pointLight.intensity = 1.0f;
			pointLight.radius = 3.0f;
			switch (rarity) {
			case static_cast<int>(Rarity::Common):
				pointLight.color = { 0.5f,0.5f,0.5f,1.0f };
				break;
			case static_cast<int>(Rarity::Rare):
				pointLight.color = { 0.1f,0.1f,0.7f,1.0f };
				break;
			case static_cast<int>(Rarity::Epic):
				pointLight.color = { 0.8f,0.1f,0.8f,1.0f };
				break;
			case static_cast<int>(Rarity::Legendary):
				pointLight.color = { 1.0f,0.8f,0.0f,1.0f };
				break;
			}
			pointLight.color = { 1.0f,0.8f,0.0f,0.5f };
		}
	}

	isForSale_ = isForSale;
	if (isForSale_) {
		price_ = 100;

		coinIcon_ = asset.LoadSprite("Resources/Items/Coin.png");
		coinIcon_->SetSize({ iconSize_, iconSize_ });
		coinIcon_->SetPivot({ 0.5f,0.5f });

		for (int i = 0; i < numbers_.size(); ++i) {
			numbers_[i] = asset.LoadSprite("Resources/Text/Numbers.png");
			numbers_[i]->SetSize({ digitSize_, digitSize_ });
			numbers_[i]->SetPivot({ 0.5f,0.5f });
		}
	}
}

WorldItem::~WorldItem() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	if (rarity_ > Rarity::None && lightIndex_ >= 0) {
		light.RemovePointLight(lightIndex_);
	}
}

void WorldItem::Draw() {
	if (isForSale_) {
		auto& ctx = GameContext::GetInstance();
		auto& render = ctx.Render();
		auto camera = render.GetCamera();

		// スクリーン変換
		Vector4 clipPos = TransformVector(Vector4(position_.x, 0.5f, position_.y + valueOffsetY_, 1.0f), (camera->viewMatrix_ * camera->projectionMatrix_));

		Vector3 ndc;
		ndc.x = clipPos.x / clipPos.w;
		ndc.y = clipPos.y / clipPos.w;
		ndc.z = clipPos.z / clipPos.w;

		Vector2 window = ctx.GetRenderWindowSize();
		Vector2 pos;
		pos.x = (ndc.x + 1.0f) * 0.5f * window.x;
		pos.y = (1.0f - ndc.y) * 0.5f * window.y;

		int price = price_;

		// 桁数
		int digitCount = 1;
		int temp = price;
		while (temp >= 10) {
			temp /= 10;
			digitCount++;
		}
		float totalWidth = numSpacing_ * (digitCount + 1);

		// 左端
		float left = pos.x - totalWidth * 0.5f;

		// アイコン
		coinIcon_->SetPosition({ left, pos.y });
		render.DrawSprite(coinIcon_.get());

		// 数字開始位置
		float numberX = left + numSpacing_;

		for (int i = 0; i < digitCount; ++i) {
			int digit = price % 10;
			price /= 10;

			numbers_[i]->SetPosition({ left + numSpacing_ * (digitCount - i), pos.y });
			numbers_[i]->SetTextureRect(digitWidth_ * digit, 0, digitWidth_, digitWidth_);
			render.DrawSprite(numbers_[i].get());
		}
	}
}

void WorldItem::MoveToPlayer(Vector2 pos) {
	position_ += Normalize(pos - position_) * moveSpeed_;
	moveSpeed_ += 0.001f;
}

void WorldItem::Erase() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	// 落ちているアイテムの削除
	isDead_ = true;
}
