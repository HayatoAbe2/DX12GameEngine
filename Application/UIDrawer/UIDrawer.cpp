#include "UIDrawer.h"
#include "Player/Player.h"
#include "Weapon/Weapon.h"
#include "Weapon/Weapons/AssaultRifle.h"
#include "Weapon/Weapons/Pistol.h"
#include "Weapon/Weapons/Shotgun.h"
#include "Weapon/Weapons/FireBall.h"
#include "Weapon/Weapons/Wavegun.h"

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

	equipAssaultRifle_ = asset.LoadSprite("Resources/Control/equipmentAssaultRifle.png");
	equipPistol_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipShotgun_ = asset.LoadSprite("Resources/Control/equipmentShotgun.png");
	equipFireBall_ = asset.LoadSprite("Resources/Control/equipmentSpellbook.png");
	equipWavegun_ = asset.LoadSprite("Resources/Control/equipmentWavegun.png");

	for (int i = 0; i < 3; ++i) {
		enchantDamage_[i] = (asset.LoadSprite("Resources/UI/Enchants/damageIncrease.png"));
		enchantBulletSize_[i] = (asset.LoadSprite("Resources/UI/Enchants/increaseBulletSize.png"));
		enchantBulletSpeed_[i] = (asset.LoadSprite("Resources/UI/Enchants/increaseBulletSpeed.png"));
		enchantBulletSpeed_[i] = (asset.LoadSprite("Resources/UI/Enchants/increaseFireRate.png"));
		enchantFireRate_[i] = (asset.LoadSprite("Resources/UI/Enchants/increaseKnockback.png"));
		enchantKnockback_[i] = (asset.LoadSprite("Resources/UI/Enchants/extraBullet.png"));
		enchantExtraBullet_[i] = (asset.LoadSprite("Resources/UI/Enchants/IncreaseMovespeed.png"));
		enchantMoveSpeed_[i] = (asset.LoadSprite("Resources/UI/Enchants/damageResist.png"));
		enchantResist_[i] = (asset.LoadSprite("Resources/UI/Enchants/damageIncrease.png"));// 未使用
	}

	equipment_ = equipPistol_.get();
	equipment_->SetSize({ 120,120 });
	equipment_->SetPosition({ 640 - 60,710 - 160 });
	/*equipment2_ = asset.LoadSprite("Resources/Control/equipmentPistol.png");
	equipment2_->SetSize({ 80,80 });
	equipment2_->SetPosition({ 640 - 200,710 - 100 });*/

	for (int i = 0; i < 3; ++i) {
		enchants_[i] = enchantDamage_[i].get();
		enchants_[i]->SetSize({ 240,40 });
		enchants_[i]->SetPosition({ 640 + 80,float(710 - 180 + i * 40) });
	}

#pragma endregion

	// 操作
	control_ = asset.LoadSprite("Resources/Control/leftClick.png");
	control_->SetSize(controlUIData_.size);
	control_->SetPosition(controlUIData_.pos);

	dashControl_ = asset.LoadSprite("Resources/Control/dash.png");
	dashControl_->SetSize(dashUIData_.size);
	dashControl_->SetPosition(dashUIData_.pos);

	reloadBar_ = asset.LoadSprite("Resources/Debug/white1x1.png");
	reloadBar_->SetSize({ reloadBarData_.size });
	reloadBar_->SetPosition({ reloadBarData_.pos });
	reloadBarBack_ = asset.LoadSprite("Resources/Debug/white1x1.png");
	reloadBarBack_->SetSize({ reloadBarData_.size });
	reloadBarBack_->SetPosition({ reloadBarData_.pos });
}

void UIDrawer::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	if (input.IsRelease(DIK_F)) { UpdatePlayerUI(); }

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

	// 装備
	auto weapon = player_->GetWeapon();
	if (weapon != nullptr) {
		render.DrawSprite(control_.get());

		// エンチャント
		for (int i = 0; i < static_cast<int>(weapon->GetStatus().rarity); ++i) {
			render.DrawSprite(enchants_[i]);
		}

		render.DrawSprite(equipment_);
	}

	auto subWeapon = player_->GetSubWeapon();
	if (subWeapon != nullptr) {
		render.DrawSprite(equipment2_);
	}

	// hp
	render.DrawSprite(lifeBack_.get());
	render.DrawSprite(damage_.get());
	render.DrawSprite(life_.get());
#pragma endregion

}

void UIDrawer::UpdatePlayerUI() {
	if (player_->GetWeapon() != nullptr) {
		// 所持武器レア度
		auto weapon = player_->GetWeapon();
		switch (weapon->GetStatus().rarity) {
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

		// 効果
		for (int i = 0; i < static_cast<int>(weapon->GetStatus().rarity); ++i) {
			auto enchant = weapon->GetStatus().enchants[i];
			switch (enchant) {
			case static_cast<int>(Enchants::increaseDamage):
				enchants_[i] = enchantDamage_[i].get();
				break;
			case static_cast<int>(Enchants::bigBullet):
				enchants_[i] = enchantBulletSize_[i].get();
				break;
			case static_cast<int>(Enchants::fastBullet):
				enchants_[i] = enchantBulletSpeed_[i].get();
				break;
			case static_cast<int>(Enchants::shortCooldown):
				enchants_[i] = enchantFireRate_[i].get();
				break;
			case static_cast<int>(Enchants::hardKnockback):
				enchants_[i] = enchantKnockback_[i].get();
				break;
			case static_cast<int>(Enchants::extraBullet):
				enchants_[i] = enchantExtraBullet_[i].get();
				break;
			case static_cast<int>(Enchants::moveSpeed):
				enchants_[i] = enchantMoveSpeed_[i].get();
				break;
			case static_cast<int>(Enchants::resist):
				enchants_[i] = enchantResist_[i].get();
				break;
			case static_cast<int>(Enchants::avoid):
				enchants_[i] = enchantDamage_[i].get();
				break;
			}
			enchants_[i]->SetSize({240,40});
			enchants_[i]->SetPosition({640 + 80,float(710 - 180 + i * 40)});
		}

		// 再装填ゲージ
		float reloadRate = float(weapon->GetReloadTimer()) / float(weapon->GetStatus().reloadTime);
		reloadBar_->SetSize({ reloadBarData_.size.x * reloadRate,reloadBarData_.size.y });

		// 二個目

		//if (player_->GetSubWeapon() != nullptr) {
		//	auto subWeapon = player_->GetSubWeapon();
		//	// 所持武器レア度
		//	switch (subWeapon->GetStatus().rarity) {
		//	case static_cast<int>(Rarity::Common):
		//		equipment2_->SetColor({ 0.5f,0.5f,0.5f,1.0f });
		//		break;
		//	case static_cast<int>(Rarity::Rare):
		//		equipment2_->SetColor({ 0.1f,0.1f,0.7f,1.0f });
		//		break;
		//	case static_cast<int>(Rarity::Epic):
		//		equipment2_->SetColor({ 0.8f,0.1f,0.8f,1.0f });
		//		break;
		//	case static_cast<int>(Rarity::Legendary):
		//		equipment2_->SetColor({ 1.0f,0.8f,0.0f,1.0f });
		//		break;
		//	}

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

void UIDrawer::UpdateStageUI() {
}
