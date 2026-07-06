#include "EquipmentUI.h"
#include "Character/Player/Player.h"
#include "Weapon/Weapon.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"
#include "Weapon/Weapons/OrbitStaff.h"
#include "Weapon/Weapons/AccelGun.h"
#include "Weapon/Weapons/ChargeGun.h"
#include "Weapon/Weapons/Sniper.h"
#include "Weapon/Weapons/Burstgun.h"

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
	shootUIKey = asset.LoadSprite("Resources/Control/KeyboardAndMouse.png");
	shootUIKey->SetSize(shootUIData_.size);
	shootUIKey->SetPosition(shootUIData_.pos);
	shootUIKey->SetTextureRect(64 * 14, 64 * 2, 64, 64);
	swapUIKey = asset.LoadSprite("Resources/Control/KeyboardAndMouse.png");
	swapUIKey->SetSize(swapUIData_.size);
	swapUIKey->SetPosition(swapUIData_.pos);
	swapUIKey->SetTextureRect(64 * 5, 64 * 3, 64, 64);
	shootUIPad = asset.LoadSprite("Resources/Control/XboxController.png");
	shootUIPad->SetSize(shootUIData_.size);
	shootUIPad->SetPosition(shootUIData_.pos);
	shootUIPad->SetTextureRect(64 * 7, 64 * 2, 64, 64);
	swapUIPad = asset.LoadSprite("Resources/Control/XboxController.png");
	swapUIPad->SetSize(swapUIData_.size);
	swapUIPad->SetPosition(swapUIData_.pos);
	swapUIPad->SetTextureRect(64 * 4, 64 * 8, 64, 64);

	ammo_ = asset.LoadSprite("Resources/Debug/White1x1.png");
	ammo_->SetSize({150, 150});
	ammo_->SetPosition({1100, 530});
	ammo2_ = asset.LoadSprite("Resources/Debug/White1x1.png");
	ammo2_->SetSize({85, 85});
	ammo2_->SetPosition({1165, 433 });
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
		}else if(dynamic_cast<OrbitStaff*>(weapon)) {
			equipment_ = equipFireBall_.get();
		} else if (dynamic_cast<ChargeGun*>(weapon)) {
			equipment_ = equipShotgun_.get();
		} else if (dynamic_cast<AccelGun*>(weapon)) {
			equipment_ = equipPistol_.get();
		} else if (dynamic_cast<Sniper*>(weapon)) {
			equipment_ = equipAssaultRifle_.get();
		} else if (dynamic_cast<Burstgun*>(weapon)) {
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
			} else if (dynamic_cast<OrbitStaff*>(weapon2)) {
				equipment2_ = equipFireBall2_.get();
			} else if (dynamic_cast<ChargeGun*>(weapon2)) {
				equipment2_ = equipShotgun2_.get();
			} else if (dynamic_cast<AccelGun*>(weapon2)) {
				equipment2_ = equipPistol2_.get();
			} else if (dynamic_cast<Sniper*>(weapon2)) {
				equipment2_ = equipAssaultRifle2_.get();
			} else if (dynamic_cast<Burstgun*>(weapon2)) {
				equipment2_ = equipWavegun2_.get();
			}

			equipment2_->SetSize({ 85, 85 });
			equipment2_->SetPosition({ 1165, 433 }); 

			ammo2_->SetSize({ 85 * weapon2->GetCharge() / weapon2->GetData().stats.maxCharge, 85 });
			ammo2_->SetColor({ 1,1,1,0.5f });
		}

		
		ammo_->SetSize({ 150 * weapon->GetCharge() / weapon->GetData().stats.maxCharge, 150 });
		ammo_->SetColor({ 1,1,1,0.5f });
	}
}

void EquipmentUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& render = ctx.Render();

	// 装備
	auto weapon = player_->GetWeapon();
	if (weapon != nullptr) {
		if (input.gamepad.IsConnected()) {
			if (input.gamepad.GetRTrigger() >= 0.2f) {
				shootUIPad->SetTextureRect(64 * 7, 64 * 2, 64, 64);
			} else {
				shootUIPad->SetTextureRect(64 * 8, 64 * 2, 64, 64);
			}
			render.DrawSprite(shootUIPad.get());
		} else {
			render.DrawSprite(shootUIKey.get());
		}
		render.DrawSprite(equipment_);
		render.DrawSprite(ammo_.get());
	}

	auto subWeapon = player_->GetSubWeapon();
	if (subWeapon != nullptr) {
		if (input.gamepad.IsConnected()) {
			if (input.gamepad.IsPress(XINPUT_GAMEPAD_B)) {
				swapUIPad->SetTextureRect(64 * 4, 64 * 8, 64, 64);
			} else {
				swapUIPad->SetTextureRect(64 * 5, 64 * 8, 64, 64);
			}
			render.DrawSprite(swapUIPad.get());
		} else {
			render.DrawSprite(swapUIKey.get());
		}
		render.DrawSprite(equipment2_);
		render.DrawSprite(ammo2_.get());
	}
}
