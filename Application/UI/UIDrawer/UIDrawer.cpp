#include "UIDrawer.h"
#include "Player/Player.h"

void UIDrawer::Initialize(Player* player) {
	player_ = player;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

#pragma region PlayerUI
	life_ = asset.LoadSprite("Resources/UI/gauge.png");
	life_->SetSize({ 290,68 });
	life_->SetPosition({ 10,10 });
	damage_ = asset.LoadSprite("Resources/Debug/white1x1.png");
	damage_->SetSize({ 290,68 });
	damage_->SetPosition({ 10,10 });
	lifeBack_ = asset.LoadSprite("Resources/UI/gauge.png");
	lifeBack_->SetSize({ 290,68 });
	lifeBack_->SetPosition({ 10,10 });
	lifeBack_->SetColor({ 0.2f,0.2f,0.2f,1 });

#pragma endregion
	dashControl_ = asset.LoadSprite("Resources/Control/dash.png");
	dashControl_->SetSize(dashUIData_.size);
	dashControl_->SetPosition(dashUIData_.pos);
}

void UIDrawer::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	if (input.gamepad.IsConnected()) {
		if (input.keyboard.IsRelease(DIK_F)) { UpdatePlayerUI(); }
	} else {
		if (input.gamepad.IsRelease(XINPUT_GAMEPAD_A)) { UpdatePlayerUI(); }
	}

	// hp
	float hpRate = player_->GetHP() / player_->GetMaxHP();
	life_->SetTextureRect(0, 0, hpRate * 290, 68);
	life_->SetSize({ hpRate * 290,68 });

	float hp = player_->GetHP();
	if (hp != whiteLife_) {
		if (hp >= whiteLife_) {
			whiteLife_ = hp;
		} else {
			whiteLife_ -= whiteGaugeSpeed_;
		}

		float preHPRate = whiteLife_ / player_->GetMaxHP();
		damage_->SetTextureRect(0, 0, preHPRate * 290, 68);
		damage_->SetSize({ preHPRate * 290,68 });
	}
}

void UIDrawer::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// プレイヤーUI
#pragma region PlayerUI
	// 操作
	render.DrawSprite(dashControl_.get());

	// hp
	render.DrawSprite(lifeBack_.get());
	render.DrawSprite(damage_.get());
	render.DrawSprite(life_.get());
#pragma endregion

}

void UIDrawer::UpdatePlayerUI() {
}

void UIDrawer::UpdateStageUI() {
}
