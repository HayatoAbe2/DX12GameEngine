#include "EquipmentUI.h"
#include "Player/Player.h"
#include "Weapon/Weapon.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"

void EquipmentUI::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	equipAssaultRifle_ = asset.LoadSprite("Resources/Control/equipmentAssaultRifle.png");
	equipPistol_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipShotgun_ = asset.LoadSprite("Resources/Control/equipmentShotgun.png");
	equipFireBall_ = asset.LoadSprite("Resources/Control/equipmentSpellbook.png");
	equipWavegun_ = asset.LoadSprite("Resources/Control/equipmentWavegun.png");

	equipment_ = equipPistol_.get();
	equipment_->SetSize({ 120,120 });
	equipment_->SetPosition({ 640 - 60,710 - 160 });
	/*equipment2_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipment2_->SetSize({ 80,80 });
	equipment2_->SetPosition({ 640 - 200,710 - 100 });*/

	// 操作
	control_ = asset.LoadSprite("Resources/Control/leftClick.png");
	control_->SetSize(controlUIData_.size);
	control_->SetPosition(controlUIData_.pos);


}

void EquipmentUI::Update(Player* player) {
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
		equipment_->SetSize({ 120, 120 });
		equipment_->SetPosition({ 640 - 60, 710 - 160 });

		// 二個目

		//	// 武器アイコン
		//	if (dynamic_cast<AssaultRifle*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2AssaultRifle.png");
		//	} else if (dynamic_cast<Pistol*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Pistol.png");
		//	} else if (dynamic_cast<Shotgun*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Shotgun.png");
		//	} else if (dynamic_cast<FireBall*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Spellbook.png");
		//	} else if (dynamic_cast<Wavegun*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Wavegun.png");
		//	}

		//	if (dynamic_cast<AssaultRifle*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2AssaultRifle.png");
		//	} else if (dynamic_cast<Pistol*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Pistol.png");
		//	} else if (dynamic_cast<Shotgun*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Shotgun.png");
		//	} else if (dynamic_cast<FireBall*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Spellbook.png");
		//	} else if (dynamic_cast<Wavegun*>(subWeapon)) {
		//		equipment2_ = asset.LoadSprite("Resources/Control/equipment2Wavegun.png");
		//	}

		//	equipment2_->SetSize({ 120, 120 });
		//	equipment2_->SetPosition({ 640 - 60, 710 - 160 });
		//}
	}
}

void EquipmentUI::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// 装備
	auto weapon = player_->GetWeapon();
	if (weapon != nullptr) {
		render.DrawSprite(control_.get());
		render.DrawSprite(equipment_);
	}

	auto subWeapon = player_->GetSubWeapon();
	if (subWeapon != nullptr) {
		//render.DrawSprite(equipment2_);
	}

}
