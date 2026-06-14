#include "EquipmentUI.h"
#include "Player/Player.h"
#include "Weapon/Weapon.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"

void EquipmentUI::Initialize(Player* player) {
	player_ = player;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	equipAssaultRifle_ = asset.LoadSprite("Resources/Control/equipmentAssaultRifle.png");
	equipPistol_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipShotgun_ = asset.LoadSprite("Resources/Control/equipmentShotgun.png");
	equipFireBall_ = asset.LoadSprite("Resources/Control/equipmentSpellbook.png");
	equipWavegun_ = asset.LoadSprite("Resources/Control/equipmentWavegun.png");

	equipAssaultRifle2_ = asset.LoadSprite("Resources/Control/equipmentAssaultRifle.png");
	equipPistol2_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipShotgun2_ = asset.LoadSprite("Resources/Control/equipmentShotgun.png");
	equipFireBall2_ = asset.LoadSprite("Resources/Control/equipmentSpellbook.png");
	equipWavegun2_ = asset.LoadSprite("Resources/Control/equipmentWavegun.png");

	equipment_ = equipPistol_.get();
	equipment2_ = equipShotgun_.get();

	// 操作
	shootUI = asset.LoadSprite("Resources/Control/KeyboardAndMouse.png");
	shootUI->SetSize(shootUIData_.size);
	shootUI->SetPosition(shootUIData_.pos);
	shootUI->SetTextureRect(64 * 14, 64 * 2, 64, 64);
	swapUI = asset.LoadSprite("Resources/Control/KeyboardAndMouse.png");
	swapUI->SetSize(swapUIData_.size);
	swapUI->SetPosition(swapUIData_.pos);
	swapUI->SetTextureRect(64 * 5, 64 * 3, 64, 64);

}

void EquipmentUI::Update() {
	if (player_->GetWeapon() != nullptr) {
		// 所持武器レア度
		auto weapon = player_->GetWeapon();
		switch (weapon->GetData().rarity) {
		case static_cast<int>(Rarity::Common):
			equipment_->SetColor({ 0.5f,0.5f,0.5f,1.0f });
			break;
		case static_cast<int>(Rarity::Rare):
			equipment_->SetColor({ 0.1f,0.1f,0.7f,1.0f });
			break;
		case static_cast<int>(Rarity::Epic):
			equipment_->SetColor({ 0.8f,0.1f,0.8f,1.0f });
			break;
		case static_cast<int>(Rarity::Legendary):
			equipment_->SetColor({ 1.0f,0.8f,0.0f,1.0f });
			break;
		}

		// 武器アイコン
		if (dynamic_cast<AssaultRifle*>(weapon)) {
			equipment_ = equipAssaultRifle_.get();
		} else if (dynamic_cast<Pistol*>(weapon)) {
			equipment_ = equipPistol_.get();
		} else if (dynamic_cast<Shotgun*>(weapon)) {
			equipment_ = equipShotgun_.get();
		} else if (dynamic_cast<FireBall*>(weapon)) {
			equipment_ = equipFireBall_.get();
		} else if (dynamic_cast<Wavegun*>(weapon)) {
			equipment_ = equipWavegun_.get();
		}
		equipment_->SetSize({ 150, 150 });
		equipment_->SetPosition({ 1100, 530 });

		auto weapon2 = player_->GetSubWeapon();
		if (weapon2 != nullptr) {
			// 二個目
			switch (weapon2->GetData().rarity) {
			case static_cast<int>(Rarity::Common):
				equipment2_->SetColor({ 0.5f,0.5f,0.5f,1.0f });
				break;
			case static_cast<int>(Rarity::Rare):
				equipment2_->SetColor({ 0.1f,0.1f,0.7f,1.0f });
				break;
			case static_cast<int>(Rarity::Epic):
				equipment2_->SetColor({ 0.8f,0.1f,0.8f,1.0f });
				break;
			case static_cast<int>(Rarity::Legendary):
				equipment2_->SetColor({ 1.0f,0.8f,0.0f,1.0f });
				break;
			}

			// 武器アイコン
			if (dynamic_cast<AssaultRifle*>(weapon2)) {
				equipment2_ = equipAssaultRifle2_.get();
			} else if (dynamic_cast<Pistol*>(weapon2)) {
				equipment2_ = equipPistol2_.get();
			} else if (dynamic_cast<Shotgun*>(weapon2)) {
				equipment2_ = equipShotgun2_.get();
			} else if (dynamic_cast<FireBall*>(weapon2)) {
				equipment2_ = equipFireBall2_.get();
			} else if (dynamic_cast<Wavegun*>(weapon2)) {
				equipment2_ = equipWavegun2_.get();
			}

			equipment2_->SetSize({ 85, 85 });
			equipment2_->SetPosition({ 1165, 433 });
		}
	}
}

void EquipmentUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// 装備
	auto weapon = player_->GetWeapon();
	if (weapon != nullptr) {
		render.DrawSprite(shootUI.get());
		render.DrawSprite(equipment_);
	}

	auto subWeapon = player_->GetSubWeapon();
	if (subWeapon != nullptr) {
		render.DrawSprite(swapUI.get());
		render.DrawSprite(equipment2_);
	}
}
