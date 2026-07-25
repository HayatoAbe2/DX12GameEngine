#include "WorldItem.h"
#include "GameCommon.h"

WorldItem::WorldItem(Vector3 pos, Rarity rarity) {
	position_ = ToXZ(pos);
	rarity_ = rarity;

	if (rarity == Rarity::None) return;

	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	// アイテム強調ライトの追加
	lightIndex_ = light.AddPointLight();
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

WorldItem::~WorldItem() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	light.RemovePointLight(lightIndex_);
}

void WorldItem::Erase() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	// 落ちているアイテムの削除
	isDead_ = true;

	// ライト削除
	light.RemovePointLight(lightIndex_);
}
