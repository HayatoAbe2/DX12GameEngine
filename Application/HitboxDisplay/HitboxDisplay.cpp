#include "HitboxDisplay.h"

HitboxDisplay::HitboxDisplay(Team team) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	primitive_ = asset.CreatePrimitive(asset.CreateMaterial(asset.LoadTexture("Resources/Debug/gradationLine.png")), PrimitiveShape::Ring);
	primitive_->transform_.rotate.x = std::numbers::pi_v<float> / 2.0f;

	MaterialData data = primitive_->GetMaterial()->GetData();
	data.enableLighting = false;
	switch(team) {
	case Team::Player:
		data.color = {0,0,1,1};
		break;

	case Team::Enemy:
		data.color = { 1,0,0,1 };
	}
	primitive_->GetMaterial()->SetData(data);
}

void HitboxDisplay::Update(float radius, Vector2 pos) {
	primitive_->transform_.scale = { radius, radius, 1.0f };
	primitive_->transform_.translate = { pos.x, kHeight, pos.y };
}

void HitboxDisplay::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	render.DrawPrimitive(primitive_.get());
}
