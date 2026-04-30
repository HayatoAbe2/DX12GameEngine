#include "Item.h"
#include "GameCommon.h"
#include <numbers>
#include "Weapon/Weapons/FireBall.h"

Item::~Item() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	light.RemovePointLight(lightIndex_);
}

Item::Item(std::unique_ptr<Weapon> weapon, Vector3 pos, Rarity rarity) {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	weapon_ = std::move(weapon);
	weapon_->GetWeaponModel()->SetTranslate(pos);
	if (weapon_ && dynamic_cast<FireBall*>(weapon_.get())) {
	} else {
		weapon_->GetWeaponModel()->SetRotate({ 0,float(std::numbers::pi / 2.0f),0 });
	}

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
}

void Item::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawModel(weapon_->GetWeaponModel(), camera);
}

void Item::Erase() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	// 落ちているアイテムの削除
	isDead_ = true;

	// ライト削除
	light.RemovePointLight(lightIndex_);
}
